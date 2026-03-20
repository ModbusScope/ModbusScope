# Concept: Schema-Based Adapter Configuration

## Purpose

This document describes the architectural concept for migrating an existing application to support schema-driven adapter configuration. It is intended as input for a code agent tasked with producing a concrete migration plan for a specific codebase.

---

## Background

The application manages one or more adapters. Adapters are external processes that communicate with the core application over stdio using JSON-RPC 2.0 with Content-Length framing. The existing Modbus adapter serves as the reference implementation and its interface is the canonical baseline.

The goal of this migration is to make all adapters self-describing, so that the core application no longer needs built-in knowledge of any specific adapter's configuration structure, validation rules, or runtime behavior.

---

## Core Architectural Principle

Responsibility is divided strictly between the core application and adapters:

**The core application is responsible for:**
- Managing the adapter process lifecycle
- Persisting configuration on behalf of adapters
- Delivering stored configuration to adapters at startup
- Generating configuration UI from adapter-supplied schemas
- Treating adapter configuration as opaque data it does not interpret

**Each adapter is responsible for:**
- Describing its own configuration schema
- Providing default configuration values
- Validating configuration it receives
- Migrating configuration between versions
- Declaring its own runtime capabilities

The core must never contain adapter-specific validation logic or hardcoded knowledge of adapter configuration fields.

---

## Adapter Session Lifecycle

Every adapter session follows this sequence:

```
adapter process started
adapter.initialize
adapter.describe
adapter.configure
adapter.start
```

This sequence is mandatory and ordered. The `adapter.describe` call is not optional — it is the mechanism by which the core learns everything it needs before proceeding.

### Why `adapter.describe` is required before `adapter.configure`

- The core needs the schema to know what a valid configuration looks like
- The core needs the defaults to construct an initial configuration for a first-time setup
- The core needs the `configVersion` to determine whether a stored configuration requires migration before delivery
- The core needs the capabilities to know how to handle configuration changes at runtime (hot reload vs restart)

---

## The `adapter.describe` Response

This is the central data structure of the entire pattern. It must contain:

| Field | Purpose |
|---|---|
| `name` | Adapter identifier |
| `version` | Adapter software version |
| `configVersion` | Current configuration schema version |
| `schema` | JSON Schema–compatible description of the configuration object |
| `defaults` | Default configuration values conforming to the schema |
| `capabilities` | Optional runtime behavior flags |

### Schema

The schema describes all configuration fields: their types, constraints, required status, and optional UI hints. It follows JSON Schema conventions. The core uses this schema to generate configuration UI dynamically, so it must be complete and accurate.

### Defaults

Defaults provide a usable starting configuration for a freshly instantiated adapter. They must conform to the schema. The core applies defaults when no stored configuration exists for an adapter instance.

### Capabilities

Capabilities inform the core how to handle runtime configuration changes. Key examples:

- `supportsHotReload` — configuration can be updated without restarting the adapter
- `requiresRestartOn` — list of specific fields that require a restart if changed
- `supportsBatching` — adapter supports batch read operations

Capabilities must not affect configuration semantics. They only affect how the core manages the adapter lifecycle around configuration changes.

---

## Configuration Storage

The core application owns and persists all adapter configuration. Adapters must not persist configuration independently.

Configuration is stored per adapter instance, identified by adapter ID. Each adapter configuration object must include a `version` field indicating which configuration schema version it was written against.

Example structure:

```
{
  "core": { ... },
  "adapters": {
    "<adapterId>": {
      "version": <N>,
      ... adapter-specific fields ...
    }
  }
}
```

---

## Configuration Delivery

After `adapter.describe`, the core:

1. Loads any stored configuration for this adapter instance
2. If no stored configuration exists, uses the defaults from `adapter.describe`
3. If the stored configuration version is older than `configVersion`, the adapter will handle migration during `adapter.configure`
4. Sends the configuration via `adapter.configure`

The adapter receives the configuration, validates it, migrates it if necessary, and applies it internally. The adapter returns the normalized (possibly migrated) configuration so the core can persist the updated version.

---

## Configuration Versioning and Migration

Every configuration object includes a `version` field. The adapter declares its current `configVersion` in the `adapter.describe` response.

If the delivered configuration version is older than the current version, the adapter performs sequential migration steps internally (v1→v2, v2→v3, etc.) and returns the migrated result. The core persists the migrated configuration so future sessions do not repeat the migration.

Migration must preserve user intent where possible.

---

## Validation

Validation is split into two layers:

**Core validation** handles only structural concerns: JSON syntax and basic schema structure. It does not interpret adapter-specific field semantics.

**Adapter validation** handles all semantic concerns: field value ranges, cross-field constraints, compatibility with external systems, and schema compliance. Validation errors are returned as standard JSON-RPC error responses.

The method `adapter.configure` is the primary validation point. The adapter must reject invalid configuration at this stage with a descriptive error before `adapter.start` is called.

---

## Runtime Reconfiguration

Configuration may be updated during an active session via `adapter.reconfigure`. The adapter consults its declared capabilities to determine whether the change can be applied without a restart. If a changed field is listed in `requiresRestartOn`, the adapter rejects the reconfigure request, and the core is responsible for performing a restart cycle.

---

## Dynamic UI Generation

The schema returned by `adapter.describe` is the sole input for generating configuration UI. The core maps schema field types and constraints to UI controls. Schema fields may include optional UI hints (labels, grouping, ordering) to guide this mapping.

Because UI generation is driven entirely by the schema, no code changes to the core are required when a new adapter is added or an existing adapter's configuration evolves.

---

## Output Rules

Adapters must follow strict output discipline:

- `stdout` is used exclusively for JSON-RPC messages
- `stderr` is used for all logging and diagnostics

Non-RPC output on stdout will corrupt the message framing and must never occur.

---

## Compatibility Requirements

The migration must preserve backward compatibility throughout. Specifically:

- Existing adapters that do not yet implement `adapter.describe` must continue to function during a transition period
- Schema evolution must be handled through versioning, not breaking changes
- The core must remain functional when adapters are updated independently
- New adapters added after the migration require no core changes beyond registering the adapter process

---

## Reference Implementation

The existing Modbus adapter is the canonical reference. Its JSON-RPC method signatures, field names, error codes, Content-Length framing, and lifecycle sequence define the baseline that all adapters must conform to. Any extensions introduced during this migration must be additive and must not alter the established Modbus adapter interface.
