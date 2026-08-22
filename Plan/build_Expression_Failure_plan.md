# Fix: non-fatal error handling for describeDataPoint/buildExpression

## Context

A prior code-review fix pass made `AdapterClient::onErrorReceived()` treat JSON-RPC
errors for `adapter.validateDataPoint` and `adapter.expressionHelp` as non-fatal
(the aux request is resolved/swallowed instead of killing the session). Two sibling
auxiliary RPCs — `adapter.describeDataPoint` and `adapter.buildExpression` — were
left out of that fix; this was flagged explicitly afterward as a known, separate gap.

Today, a JSON-RPC error response to either of these two methods, in any state, falls
through `onErrorReceived()`'s early-return chain to the generic fatal-error handler,
which force-kills the adapter subprocess and emits `sessionError()`. This turns a
routine, recoverable error (e.g. "these address fields don't form a valid expression")
into a full session teardown — inconsistent with how `validateDataPoint`/`expressionHelp`
already behave, and reachable any time a user interacts with the register-building UI.

Both RPCs' request-sending guards (`describeDataPoint()`, `buildExpression()`) already
allow `AWAITING_CONFIG`/`ACTIVE`/`ACTIVE_DEGRADED` — only the error-response handling is
missing. Per `docs/technical/adapter-protocol-spec.md`, `describeDataPointResult`
(`QJsonObject`) already has a documented "invalid" shape (`{"valid": false, "error": "..."}`)
that callers must already tolerate, so a JSON-RPC error can be translated into that same
shape. `buildExpressionResult` (bare `QString`) has no error-carrying capacity at all —
its correct non-fatal handling is to swallow the error, exactly mirroring the existing
`expressionHelp` pattern.

While validating this plan, checking `onErrorReceived()`'s three real consumers of these
two signals (`ExpressionsDialog`, `RegisterDialog`, `AddRegisterWidget`) confirmed that
`AddRegisterWidget` has a UI regression this fix will make reachable: its "Add" button is
disabled before sending `buildExpression` and only re-enabled inside the success handler.
Today an error kills the whole session (an obvious, unmissable failure), which masks this;
once errors are silently swallowed, the button will get stuck disabled with no feedback.
Per the user's direction, this is **out of scope for this change** — it will be written up
as a follow-up markdown for later action, not fixed now.

## Implementation

### 1. `src/ProtocolAdapter/adapterclient.cpp` — `AdapterClient::onErrorReceived()`

Insert two new early-return blocks, ordered to match `handleLifecycleResponse()`'s method
order (describeDataPoint → validateDataPoint → buildExpression → expressionHelp). Insert
the `describeDataPoint` block immediately before the existing `validateDataPoint` block,
and the `buildExpression` block between the existing `validateDataPoint` and `expressionHelp`
blocks:

```cpp
if (method == QStringLiteral("adapter.describeDataPoint") &&
    (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
{
    if (_pendingAuxRequests.value(method, -1) == id)
    {
        _pendingAuxRequests.remove(method);
        emit describeDataPointResult(QJsonObject{ { "valid", false }, { "error", errorMsg } });
    }
    return;
}
```

```cpp
if (method == QStringLiteral("adapter.buildExpression") &&
    (_state == State::AWAITING_CONFIG || _state == State::ACTIVE || _state == State::ACTIVE_DEGRADED))
{
    if (_pendingAuxRequests.value(method, -1) == id)
    {
        _pendingAuxRequests.remove(method);
    }
    return;
}
```

No `"fields"` key in the synthesized `describeDataPoint` object — matches the documented
invalid-shape exactly, and the only consumer (`ExpressionsDialog::handleDescribeDataPointResult()`)
only reads `"description"`, which is already absent on the documented invalid shape.

No header (`adapterclient.h`) changes needed: `describeDataPointResult`'s doc comment
already documents the `error` field generically; `buildExpressionResult`'s doc comment
already says nothing about failure, consistent with `expressionHelpResult`'s (whose
existing swallow-on-error behavior is likewise undocumented there).

### 2. `tests/ProtocolAdapter/tst_adapterclient.h` — new test-slot declarations

- `void describeDataPointErrorIsNonFatal();` — insert after `describeDataPointInWrongStateIgnored()`, before `validateDataPointValid()`.
- `void buildExpressionErrorIsNonFatal();` — insert after `buildExpressionOmitsDefaultDeviceId()`, before `expressionHelpRequestAndResponse()`.

### 3. `tests/ProtocolAdapter/tst_adapterclient.cpp` — new/updated tests

**`describeDataPointErrorIsNonFatal()`** (insert after `describeDataPointInWrongStateIgnored()`), following the `expressionHelpErrorIsNonFatal()` template (drive to `AWAITING_CONFIG`, call, inject error, assert no `sessionError`, assert translated result, then a follow-up successful call proves the session and aux-request bookkeeping still work):

```cpp
void TestAdapterClient::describeDataPointErrorIsNonFatal()
{
    auto mockOwned = std::make_unique<MockAdapterProcess>();
    auto* mock = mockOwned.get();
    AdapterClient client(std::move(mockOwned));

    QSignalSpy spyError(&client, &AdapterClient::sessionError);
    QSignalSpy spyDescReg(&client, &AdapterClient::describeDataPointResult);

    driveToAwaitingConfig(client, mock);

    client.describeDataPoint(QStringLiteral("${bad}"));

    QJsonObject error;
    error["code"] = -32602;
    error["message"] = QStringLiteral("Invalid address fields");
    mock->injectError(3, "adapter.describeDataPoint", error);

    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyDescReg.count(), 1);
    QJsonObject received = spyDescReg.at(0).at(0).value<QJsonObject>();
    QCOMPARE(received["valid"].toBool(), false);
    QCOMPARE(received["error"].toString(), QStringLiteral("Invalid address fields"));

    /* Session should still be usable — verify a follow-up request succeeds */
    client.describeDataPoint(QStringLiteral("${h0}"));
    mock->injectResponse(4, "adapter.describeDataPoint",
                         QJsonObject{ { "valid", true }, { "description", QStringLiteral("Holding register 0") } });
    QCOMPARE(spyDescReg.count(), 2);
    QCOMPARE(spyDescReg.at(1).at(0).value<QJsonObject>()["valid"].toBool(), true);
}
```

**`buildExpressionErrorIsNonFatal()`** (insert after `buildExpressionOmitsDefaultDeviceId()`), directly mirroring `expressionHelpErrorIsNonFatal()` since the signal has no error-carrying capacity:

```cpp
void TestAdapterClient::buildExpressionErrorIsNonFatal()
{
    auto mockOwned = std::make_unique<MockAdapterProcess>();
    auto* mock = mockOwned.get();
    AdapterClient client(std::move(mockOwned));

    QSignalSpy spyError(&client, &AdapterClient::sessionError);
    QSignalSpy spyBuild(&client, &AdapterClient::buildExpressionResult);

    driveToAwaitingConfig(client, mock);

    QJsonObject fields;
    fields["objectType"] = QStringLiteral("holding register");
    client.buildExpression(fields, QStringLiteral("16b"), 1);

    QJsonObject error;
    error["code"] = -32602;
    error["message"] = QStringLiteral("Invalid address fields");
    mock->injectError(3, "adapter.buildExpression", error);

    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyBuild.count(), 0);

    /* Session should still be usable — verify a follow-up request succeeds, proving the
       pending-aux-request bookkeeping was cleared by the error path */
    client.buildExpression(fields, QStringLiteral("16b"), 1);
    mock->injectResponse(4, "adapter.buildExpression", QJsonObject{ { "expression", QStringLiteral("${h0}") } });
    QCOMPARE(spyBuild.count(), 1);
    QCOMPARE(spyBuild.at(0).at(0).toString(), QStringLiteral("${h0}"));
}
```

**Extend `auxRequestErrorsAreNonFatalInDegradedSession()`** to also cover `describeDataPoint`/
`buildExpression` while `ACTIVE_DEGRADED`, matching its existing coverage of
`validateDataPoint`/`expressionHelp` there. Update its doc comment to mention all four
RPCs, add `spyDescReg`/`spyBuild` spies, and insert `describeDataPoint`/`buildExpression`
error round-trips (ids 4 and 6, renumbering `validateDataPoint`→5 and `expressionHelp`→7)
following the same describeDataPoint → validateDataPoint → buildExpression → expressionHelp
ordering used above, asserting `spyDescReg` gets the translated `{valid:false, error:...}`
object and `spyBuild` stays empty, alongside the existing `spyValidate`/`spyHelp`/`spyError`
assertions.

`auxRequestsWorkInDegradedSession()` (the success-path sibling) already covers all four
RPCs and needs no changes.

### 4. Follow-up documentation (not a code fix)

Create `/home/jens/.claude/plans/addregisterwidget-buildexpression-error-followup.md`
documenting:
- The issue: `AddRegisterWidget::handleResultAccept()` (src/dialogs/addregisterwidget.cpp:226-243)
  disables `btnAdd` before calling `buildExpression()` and only re-enables it inside
  `onBuildExpressionResult()` (line 245-262), which fires only on success. Once
  `buildExpression` errors become non-fatal (this change) instead of killing the whole
  session, a failed "Add" click leaves the button stuck disabled with no error shown to
  the user — recoverable only indirectly (e.g. switching the device dropdown re-triggers
  `applyDevice()`, which recalculates the button's enabled state).
- Why it wasn't fixed now: properly fixing it requires a design decision (does
  `buildExpressionResult` gain an error parameter, mirroring `validateDataPointResult`? or
  does the widget use a timeout/local flag instead?) that goes beyond the originally-scoped
  `onErrorReceived` gap, and was explicitly deferred by the user.
- Confirmed NOT affected for comparison: `RegisterDialog::requestDefaultExpression()`
  (registerdialog.cpp:215-240) calls `buildExpression()` for a background default-value
  refresh and doesn't gate any UI state on the response, so a swallowed error there is a
  no-op. `ExpressionsDialog::handleDescribeDataPointResult()` (expressionsdialog.cpp:322-333)
  only reads `"description"`, which is already absent on the invalid/error shape, so it's
  unaffected by the `describeDataPoint` half of this fix.
- The three options presented to the user, for reference when they pick this up:
  1. Fix core gap only, note this separately (what we're doing now).
  2. Re-enable the button on any non-success outcome (e.g. a short timeout after the
     request, or an internal "request settled" flag independent of success/failure).
  3. Widen `buildExpressionResult`'s signature to carry success/failure explicitly
     (mirroring `validateDataPointResult(bool, QString)`), updating all three call sites.

## Verification

1. Build: `mkdir -p build && cmake -GNinja -S . -B build && ninja -C build` — must be
   clean (project uses `-Wall -Wextra -Werror`).
2. Test: `ctest --test-dir build --output-on-failure` — all tests must pass, including the
   new/updated `TestAdapterClient` cases.
3. Quality: run `clang-format -i`, `./scripts/run_clang_tidy.sh`, and
   `./scripts/run_clazy.sh` against `src/ProtocolAdapter/adapterclient.cpp` and
   `tests/ProtocolAdapter/tst_adapterclient.cpp` — no violations.
4. Confirm the follow-up markdown file was created in `/home/jens/.claude/plans/` with the
   content above (this is documentation, not code — no build/test implication).
5. Per `CLAUDE.md`, after source changes run build → test → quality → the `code-reviewer`
   agent, in that order, before considering the work done.
