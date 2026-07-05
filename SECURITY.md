# Security Policy

This document describes how to report a vulnerability in ModbusScope and what you can
expect from us (coordinated vulnerability disclosure).

## Scope

This policy covers:

- The **ModbusScope desktop application**, including the bundled Modbus adapter
- The **release artifacts** published on the [releases page](https://github.com/ModbusScope/ModbusScope/releases)
  (Windows installer, standalone packages)
- Supporting services: the [modbusscope.com](https://modbusscope.com/) website and the
  update-check endpoint

Out of scope:

- Issues that require physical access to a user's machine or social engineering
- Vulnerabilities in third-party dependencies without a demonstrated impact on
  ModbusScope; report those upstream (a heads-up to us is still welcome)
- Limitations of the Modbus protocol itself: Modbus has no authentication or encryption
  by design, so protocol-level weaknesses are not vulnerabilities in ModbusScope

## Supported versions

Security fixes are provided for the latest release line and ship as patch releases,
separate from feature updates.

| Version | Supported                         |
| ------- | --------------------------------- |
| 5.0.x   | ✔                                 |
| 4.2.x   | ✔                                 |
| < 4.2   | ✘ (update to a supported release) |

## How to report a vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

Report privately through one of these channels:

- **Email (preferred):** [security@modbusscope.com](mailto:security@modbusscope.com)
- **GitHub:** [private vulnerability reporting](https://github.com/ModbusScope/ModbusScope/security/advisories/new)
  on this repository

Please include, where possible:

- The affected version(s) and platform
- Steps to reproduce, a proof of concept, or the affected file/input
- The impact you believe the issue has

## What you can expect from us

- **Acknowledgement** of your report within **48 hours**
- An **initial assessment** (severity, affected versions) within **7 days**
- Status updates while a fix is being developed
- A fix released **without undue delay**, as a free security update for all users of a
  supported version
- Credit in the published advisory, if you wish (you may also remain anonymous)

## Coordinated disclosure

We follow a coordinated disclosure process with a default window of **90 days** from the
initial report to public disclosure. The window can be shortened (fix available earlier,
or the issue is already public) or extended (a fix needs more time) by mutual
agreement.

Once a fixed version is available, we publish an advisory via
[GitHub Security Advisories](https://github.com/ModbusScope/ModbusScope/security/advisories)
including the affected version range, the fixed version, workarounds if any, and credit
to the reporter. A CVE identifier is requested where applicable.

## Safe harbor

We will not pursue legal action against researchers who, in good faith:

- Follow this policy and report through the channels above
- Make a reasonable effort to avoid privacy violations, data destruction, and disruption
  of services or other users
- Do not exploit a vulnerability beyond what is necessary to demonstrate it, and stop and
  report as soon as it is confirmed
