# Modbus terminology analysis

**Goal:** inventory every place Modbus-specific wording is visible to the user, and propose generic
replacements that read correctly regardless of which protocol adapter is active (Modbus, IEC 104, BACnet, …).

## Context: the app is already mid-migration

The codebase is not starting from zero. The `ProtocolAdapter` layer (`AdapterManager`, `AdapterClient`,
`AdapterHub`) already speaks a protocol-agnostic RPC vocabulary: `adapter.describeDataPoint`,
`buildExpression`, `Device`, `Connection`, `adapterId`. `AdapterDeviceSettings` and `DeviceConfigTab`
render device/connection settings from a JSON Schema the adapter supplies at runtime — there is no
hardcoded "Slave ID" or "Coil" anywhere in that dialog's C++. The Modbus adapter's schema
(`adapters/Modbus/modbus-adapter-spec.md`) supplies its own field labels (`"Slave ID"`, `"Object type"`,
`x-enumLabels: ["Coil", "Discrete Input", "Input Register", "Holding Register"]`) — a future BACnet or
IEC 104 adapter would supply its own labels (e.g. "Object Instance", "IOA") through the same mechanism.
**This schema-driven pattern is the correct model and needs no rewording** — it is called out below
only as the contrast case.

The problem is everything *around* that layer: the parts of the UI, C++ literals, and documentation that
were written when Modbus was the only protocol and still hardcode that assumption. Those are cataloged
below.

## Two-level model: Signal vs. Data Point

The original version of this report treated "Register" as a single term and proposed collapsing it to
one generic replacement ("Data point"). That was wrong — the app actually has **two distinct
protocol-agnostic concepts** that "Register" was ambiguously standing in for, and the codebase already
has a name for one of them:

- **Data point** — one raw `(address, deviceId)` reference read from the adapter for a single poll —
  e.g. the `${5@2}` token on its own. This is *already* an established internal concept:
  `src/communication/datapoint.h` defines a `DataPoint` class (address + device ID), and
  `ExpressionParser::dataPoints()` (`src/datahandling/expressionparser.cpp:14-20`) extracts the
  deduplicated list of these from every configured Expression before polling. The entire
  `adapter.dataPointSchema` / `adapter.describeDataPoint` / `adapter.buildExpression` RPC vocabulary
  (`src/ProtocolAdapter/adapterclient.{h,cpp}`, `src/ProtocolAdapter/adaptermanager.{h,cpp}`,
  `src/models/adapterdata.{h,cpp}`) already uses "data point" correctly at this atomic, adapter-specific
  level — untouched by this migration, and doesn't need to be.
- **Signal** — the named, colored, user-configured graph row (Name / Expression / Color / Y-Axis) that a
  user adds and sees plotted. A Signal is computed by evaluating its **Expression** — a muParser formula
  — over one or more **Data Points**, e.g. `${5@2} + ${6}` sums two Data Points into one Signal value.
  It is not itself "the result of a poll"; it's the result of evaluating an Expression.

The first pass of app-shell rewording (Phase 1) initially renamed the Signal-level UI chrome ("Register(s)"
→ menu, dialog title, buttons, error messages, default curve name) to "Data Point(s)" — which collided
with the existing atomic-level meaning and was inaccurate (a Signal can combine several Data Points, so
it isn't itself "a data point"). This was corrected: Signal-level UI text now says "Signal(s)", and
`expressionsdialog.cpp`'s "Compose Expression" table — which lists one row per raw `${...}` address found
*inside* an Expression being composed, confirmed via `_expressionChecker.addresses()` — keeps "Data
Point" as its column header, since that one *is* genuinely at the atomic level. See the corrected mapping
in section 3.

## How this was gathered

Grepped `src/**/*.ui`, `src/**/*.cpp` (`tr("...")`, `setText`, `setWindowTitle`, `setHeaderData`), CSV
export code, and `docs/manual/**/*.md` for Modbus vocabulary (register, coil, holding/input register,
discrete input, slave, master, unit ID, function code, MBC, Modbus). Every finding below is cited as
`file:line`.

---

## 1. Findings — application (C++ / `.ui`)

### "Register" / "Registers" — the primary term to replace

This is the dominant term. It is used both as the generic name for "a thing you poll and graph" (which
is protocol-agnostic in Modbus, IEC 104, and BACnet alike) and, confusingly, sometimes to mean the
Modbus-specific memory-area concept.

| Location | Current text | UI element |
| --- | --- | --- |
| `src/dialogs/mainwindow.ui:420,423` | `&Registers` / `Registers` | Menu item + toolbar action (opens the register list) |
| `src/dialogs/mainwindow.cpp:66` | `"No registers configured — click Registers to add one"` | Empty-graph overlay label |
| `src/dialogs/registerdialog.ui:14` | `Registers` | Dialog window title |
| `src/dialogs/registerdialog.ui:35` | `Add` | Button (adds a register row) |
| `src/dialogs/addregisterwidget.ui:75` | `Add register` | Wizard/window title (opens `AddRegisterWidget`) |
| `src/controllers/scopecontroller.cpp:152` | `"There are no registers in the scope list. Please select at least one register."` | Error dialog |
| `src/customwidgets/legend.cpp:36` | `"Register"` | Graph legend table column header |
| `src/dialogs/expressionsdialog.cpp:58` | `"Register"` / `"Value"` | "Compose Expression" example-input table headers |
| `src/models/graphdata.cpp:9` | `"Unknown register"` | Default curve/data-point name shown in legend, register list, and CSV header when no name is set |
| `src/dialogs/importmbcdialog.ui:14,87` / `.cpp:348-349` | `Import registers from mbc file`, `Import selected registers`, `Unimported registers`, `"Some registers are selected, but aren't imported yet."` | Import dialog title, button, warning dialog |
| `src/dialogs/quickstartdialog.ui:233` | `"Open Project → Registers to add the Modbus registers you want to monitor."` | Quick Start onboarding step 2 |
| CSV export (`docs/manual/reference/csv-format.md:8,22`, backed by `graphdata.cpp` default label) | `Register 1;Register 2;Register 3` | Default CSV column headers for unnamed curves |
| `src/ProtocolAdapter/adaptermanager.h:39,44,79,138` (doc comments only, not user-visible) | `registerExpressions`, "Register expression strings" | Doxygen comments — low priority, internal only |

**Note:** the register **list's own column headers** (`GraphDataModel::headerData`, `src/models/graphdatamodel.cpp:135-144`)
are already generic: `Color`, `Active`, `Name`, `Expression`, `Y-Axis`. Only the *dialog/menu/dock chrome
around* that table says "Register(s)".

### "Slave" / "Unit ID" — confirmed schema-driven (`adapters/Modbus/describe.json`)

No hits in `.cpp`/`.ui` source — as expected, these come from the Modbus adapter's schema, rendered
dynamically. Concretely, `adapters/Modbus/describe.json` supplies:

| Schema field | `title` shown in UI | Where rendered |
| --- | --- | --- |
| `slaveId` | **Slave ID** (tooltip: "Modbus slave ID") | Devices tab, Settings dialog |
| `int32LittleEndian` | **32-bit Little Endian** | Devices tab, Settings dialog |
| `consecutiveMax` | **Max Consecutive Registers** | Devices tab, Settings dialog |
| `objectType` (in the per-expression address form) | **Object type**, enum labels `Coil` / `Discrete Input` / `Input Register` / `Holding Register` | "Add register" popup, address-entry form |
| `deviceId` (in the per-expression address form) | **Device ID** | "Add register" popup, address-entry form |
| `baudrate`/`databits`/`parity`/`portName`/`stopbits`/`id`/`name`/`persistent`/`timeout`/`type`/`ip`/`port` | **Baud Rate**, **Data Bits**, **Parity**, **Serial Port Name**, **Stop Bits**, **Connection ID**, **Connection Name**, **Keep Connection Persistent**, **Timeout (ms)**, **Connection Type**, **IP Address**, **TCP Port** | Connections tab, Settings dialog |

This confirms the Context section's premise directly: these labels live entirely in
`adapters/Modbus/describe.json`, not in app C++/`.ui` code, so a BACnet or IEC 104 adapter shipping its
own `describe.json` would show its own field names through the exact same `SchemaFormWidget` /
`AdapterSettings` rendering path with **no app-shell changes needed**. "Master" does not appear anywhere
in the shipped UI today (Modbus 2018+ calls this role "Client" if it's ever added). These terms are,
however, pervasive in the **user manual** — see section 2.

### Leaky raw adapter id — not really a "terminology" issue, but adjacent and worth fixing alongside this work

`src/dialogs/addregisterwidget.cpp:164,179` builds a `"Protocol: %1"` label from either the bare adapter
id or `AdapterData::name()` — for the Modbus adapter this literally renders as **"Protocol: modbus"**
(adapter not yet running) or **"Protocol: modbusAdapter"** (running), because `describe.json:42` sets
`"name": "modbusAdapter"` — a machine identifier, not a display string. Similarly,
`src/dialogs/settingsdialog.cpp:56-58` labels settings pages **"Connections [modbus]"** /
**"General [modbus]"**, splicing the raw adapter id into the page title. Neither of these is a
Modbus-vs-generic wording choice — they're a missing "display name" concept on the adapter contract.
Recommendation: give adapters a proper human-readable display name (e.g. `describe.json` gains a
`displayName: "Modbus"` field, or the app titlecases known ids) so these labels read "Protocol: Modbus" /
"Connections [Modbus]" instead of leaking the internal id — worth doing in the same pass since it's the
same class of "raw protocol implementation detail surfacing in the UI" problem.

### "MBC" file import

`ModbusControl` (`.mbc`) is a real, Modbus-specific third-party config file format
(`src/MbcInterface/*`, `src/dialogs/importmbcdialog.*`). The feature itself is inherently Modbus-only —
there is no generic replacement for "import an .mbc file." Recommendation: leave the *feature name*
alone, but reword the surrounding dialog copy to say "data points" instead of "registers" (see table
above), and treat the whole feature as something that should only appear in the UI when the Modbus
adapter is active, rather than trying to generalize its name.

### Legacy project-file field name

`src/importexport/projectfiledefinitions.h:28` — `const char cSlaveIdTag[] = "slaveid";` and
`src/importexport/legacy/projectfilexmlparser.cpp` use `"slave"`/`"modbus"` as internal JSON/XML keys for
backward-compatible project-file parsing. Not rendered in the UI directly. Low priority — only worth
touching if/when the project file schema itself is revisioned, since changing on-disk keys breaks
backward compatibility with existing `.mbs` files. The **current** exporter
(`projectfilejsonexporter.cpp`) already writes fully generic keys (`adapters`, `devices`, `id`, `type`,
`settings`) — only the legacy *importer* still deals in Modbus-tagged XML/JSON, purely to keep opening
old files working. `src/importexport/legacy/presetxmlparser.cpp:31,33` similarly checks/logs the legacy
preset-file root tag `modbusscope` when import fails ("Unexpected root tag '%1', expected 'modbusscope'"
— only shown if a foreign/malformed file is opened).

### Command-line interface

`src/mainapp.cpp:75` — `--help` application description: `"Log data through the Modbus protocol"`.
`src/mainapp.cpp:80` — positional-argument help: `"Project file (.mbs) to open"`. Both are genuinely
Modbus-specific today (the app only supports Modbus), so low priority until a second adapter actually
ships — at that point the description should stop naming one protocol.

### MBC import/update tables (additional column headers not caught in the first pass)

`src/MbcInterface/mbcupdatemodel.cpp:31-37` — the "Update from MBC" comparison table headers:
`Current expression`, `Current Text`, `New Expression`, `New Text`. `src/MbcInterface/mbcregistermodel.cpp:126,130`
— row tooltips in the MBC import table: `Data type is not supported`, `Not readable`. These say
"expression"/"text", not "register", so they don't need renaming themselves — listed here only because
they live in the same dialog as the items in the mapping table below.

### Product name

"ModbusScope" itself (window title, About dialog, CSV header comment `"ModbusScope version"` at
`src/importexport/datafileexporter.cpp:277`) is the product name, not a UI label — out of scope for a
terminology pass. Renaming the product is a separate, much bigger decision and isn't included here.

---

## 2. Findings — user manual (`docs/manual/`)

The manual is written entirely from a Modbus-only perspective and would need its own pass; it's far
larger in volume than the app UI. Rough hit counts per file (occurrences of register/slave/master/coil/
modbus/discrete):

| File | Hits | Notes |
| --- | --- | --- |
| `reference/register-syntax.md` | 25 | Entire page is Modbus wire-format notation (Modicon addressing, function codes) — protocol-specific by nature, see recommendation below |
| `tutorials/first-logging-session.md` | 16 | Tutorial walks through "Slave ID", "Modbus TCP device" |
| `how-to/add-registers.md` | 12 | Title itself: "Add Modbus registers" |
| `how-to/import-mbc-file.md` | 10 | MBC-specific, expected |
| `explanation/polling-and-sample-rate.md`, `how-to/optimize-poll-rate.md` | 10, 9 | Poll-cycle explanation phrased around Modbus request/response |
| `explanation/connections-and-devices.md` | 7 | Defines "device" as "a Modbus slave" throughout |
| `reference/device-settings.md`, `reference/connection-settings.md` | 5, 2 | Document the Modbus-specific schema fields (Slave ID, byte order) |
| `intro.md` | 6 | Product description: "connects to one or more Modbus slaves…plots register values" |
| ~15 other how-to/reference pages | 1–8 each | Scattered mentions |

Representative examples:

- `docs/manual/intro.md:6` — *"ModbusScope is a graphical tool for logging and visualizing Modbus data in real time. It connects to one or more Modbus slaves over TCP or RTU, plots register values as they arrive…"*
- `docs/manual/index.rst:23` — how-to guide titled **"Add registers"**
- `docs/manual/explanation/connections-and-devices.md:11` — *"A device describes a Modbus slave sitting on that transport."*
- `docs/manual/reference/device-settings.md:14` — table row **"Slave ID"**
- `docs/manual/tutorials/first-logging-session.md:31` — *"Set **Slave ID** to `1` (the Modbus slave ID of the device)."*

**Recommendation for the manual:** treat it as a second, separate migration pass once the in-app terms
are settled (the manual should describe the UI accurately, so it has to follow, not lead). Pages that
document Modbus wire-format specifics (`register-syntax.md`, the Modicon/function-code tables) are
inherently protocol-specific reference material — those should stay Modbus-titled (e.g. rename to
"Modbus register syntax" for clarity) rather than being forced into generic language, and a parallel
page would document IEC 104/BACnet addressing once those adapters exist.

---

## 3. Suggested terminology mapping

| Modbus-specific term | Suggested generic term | Rationale |
| --- | --- | --- |
| Register / Registers (the graph row: Name/Expression/Color/Y-Axis) | **Signal** / **Signals** | The row is the result of evaluating an *Expression* over one or more *Data Points* — it isn't itself "a data point," so it needs its own name, distinct from the pre-existing atomic `DataPoint` class. "Signal" fits the product's oscilloscope framing ("…Scope") and has no collision with existing vocabulary |
| "Add register" (dialog/button) | **Add signal** | — |
| "Registers" menu/toolbar/dock title | **Signals** | — |
| "Unknown register" (default name) | **Unknown signal** | — |
| Legend table column header ("Register" in `legend.cpp`) | **Signal** | Shows the curve's `label()`/Name — Signal-level, confirmed via `graphdatamodel.cpp` calls in `legend.cpp` |
| "Compose Expression" table column header ("Register" in `expressionsdialog.cpp`) | **Data Point** (unchanged from the original proposal) | This table lists one row per raw `${...}` address found *inside* the Expression being composed (`_expressionChecker.addresses()`) — genuinely atomic-level, so "Data Point" is correct here, not "Signal" |
| CSV default column name "Register N" | **Signal N** (or reuse "Value N") | Cosmetic; changing it affects exported file compatibility — treat as a breaking change for downstream CSV consumers, needs a decision |
| "Import registers from mbc file" / "Import selected registers" | *(left as-is — see MBC section below)* | The user decided the MBC import dialog should keep "register" wording, since `.mbc` is a Modbus-only file format |
| "the Modbus registers you want to monitor" (Quick Start) | "the signals you want to monitor" | Quick Start is onboarding chrome and should not name a specific protocol |
| Slave / Slave ID | **Device** / **Device ID** (already used elsewhere in the app: `Device`, `deviceId_t`) | The app already models "device" generically; "Slave ID" is the one Modbus-specific holdout name for what's otherwise just a per-device identifier. Note IEC 104 (Common Address of ASDU) and BACnet (Device Instance) have their own equivalents — this label is properly adapter-schema-owned (see Context section), so this row is guidance for the Modbus adapter's own schema copy, not for app-shell code |
| Master | **Client** (Modbus 2020 terminology) or simply omit — the app never actually says "master" in-repo today | No code changes needed; flagged for awareness only |
| Coil / Discrete Input / Holding Register / Input Register | *(leave as Modbus adapter schema vocabulary — correct as-is)* | These are real Modbus object-type names with no generic equivalent; BACnet/IEC 104 adapters will supply their own type labels through the same schema mechanism |
| Function code | *(protocol-internal; not currently surfaced to the user)* | No action needed |
| MBC file / "Import from MBC" | *(leave as-is — real file format name and terminology)* | Reword only truly generic sentence-level copy elsewhere; the dialog itself, including its "register" wording, stays Modbus-specific per user decision |
| "Modbus TCP" / "Modbus RTU" (adapter type labels, connection dialogs) | *(leave as adapter-supplied display name)* | Already resolved via `pAdapter->name()` (`src/dialogs/adapterdevicesettings.cpp:193`) — not hardcoded |

---

## 4. Suggested phased approach

1. **App shell wording** (small, low-risk, high-visibility; done) — rename the menu/toolbar/dock/dialog
   chrome — "Registers" → "Signals" — across `mainwindow.ui`, `registerdialog.ui`,
   `addregisterwidget.ui`, and the associated `tr()`/plain-string literals in `mainwindow.cpp`,
   `scopecontroller.cpp`, `quickstartdialog.ui`, `legend.cpp`, and the default label in `graphdata.cpp`.
   `expressionsdialog.cpp`'s "Data Point" column header was left as-is (it's genuinely atomic-level, see
   section 2 above). The MBC import dialog (`importmbcdialog.cpp`/`.ui`) was intentionally left
   untouched — the user decided "register" wording is correct there, since `.mbc` is a Modbus-only file
   format. This was almost entirely string literals — no architectural change, since the underlying
   models/columns were already generic.
2. **Fix the leaky adapter id** (`addregisterwidget.cpp:164,179`, `settingsdialog.cpp:56-58`): add a
   proper display name to the adapter contract so "Protocol: modbusAdapter" and "Connections [modbus]"
   become "Protocol: Modbus" / "Connections [Modbus]". Small, self-contained, and removes the most
   visible "implementation detail leaking into the UI" instance found.
3. **CSV default column naming**: decide whether to rename the unnamed-curve default from
   `Register N` to `Data point N` — this changes exported file content, so confirm there's no
   compatibility expectation on existing tooling that parses ModbusScope CSVs before changing it.
4. **User manual**: a follow-up documentation pass, once (1) lands, to keep the manual in sync with the
   renamed UI — reword the generic pages (`intro.md`, `connections-and-devices.md`,
   `add-registers.md` → `add-data-points.md`, etc.) while leaving the Modbus-wire-format reference
   pages (`register-syntax.md`, Modicon notation tables) explicitly Modbus-scoped.
5. **Leave alone**: adapter-schema-driven labels (Coil/Discrete Input/Holding Register/Input Register,
   Slave ID, 32-bit Little Endian, Baud Rate/Parity/etc., Modbus TCP/RTU), the MBC file format name and
   its "ModbusControl project file" error text, the product name "ModbusScope", the CLI `--help` text
   (until a second adapter ships), and the legacy `.mbs`/preset XML tags (`slaveid`, `modbus`,
   `modbusscope`) — none of these have a generic replacement that wouldn't either be wrong
   (protocol-specific concepts have no protocol-agnostic equivalent) or break backward compatibility.
