# Third-Party Licenses

`bacnetAdapter` statically links the following third-party libraries. Their license texts
must accompany any distributed copy of the binary.

## bacnet-stack

- Project: <https://github.com/bacnet-stack/bacnet-stack>
- License: per-file; predominantly `GPL-2.0-or-later WITH GCC-exception-2.0` (core protocol
  stack code) and `MIT` (example/demo/skeleton code), with a handful of files under
  `Apache-2.0`, `BSD-2-Clause`, `BSD-3-Clause` and `CC-PDDC`. The GCC linking exception is
  what permits statically linking the stack into this proprietary binary without the binary
  becoming GPL. Full per-license texts for every SPDX identifier used in the vendored tree
  are kept at `libraries/bacnet-stack/license/`; a per-file breakdown can be regenerated with
  `grep -nrw SPDX --include=*.[c,h] libraries/bacnet-stack`.
- Copyright: Copyright (C) 2005 Steve Karg and the bacnet-stack contributors

```text
GNU GENERAL PUBLIC LICENSE
   Version 2, June 1991

In addition to the permissions in the GNU Library General Public License,
the Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs, and to
distribute those programs without any restriction coming from the use of
this file. (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

See `libraries/bacnet-stack/license/GPL-2.0` for the full GPL-2.0-or-later text and
`libraries/bacnet-stack/license/GCC-exception-2.0` for the full exception text.
```

```text
MIT License

Copyright (c) 2005 Steve Karg <skarg@users.sourceforge.net>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```

## libsodium

- Project: <https://libsodium.org>
- License: ISC License
- Copyright: Copyright (c) 2013-2026 Frank Denis <j at pureftpd dot org>

```text
ISC License

Copyright (c) 2013-2026
Frank Denis <j at pureftpd dot org>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
