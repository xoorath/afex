# Decisions about third party libraries

Very few libraries should be globally accessible across all modules of all applications within AFEX. When adding a new third party dependency it should be added to the most narrow library practical. The decision to wrap that dependency or make it public to consumers of a first part library is a case-by-case decision based on the complexity or usefulness of wrapping that library.

## Global third party libraries

### ICU 

ICU is a set of unicode and globalization libraries.

It is global because:

1. It is used by AFEX Core and users of AFEX Core.
2. Nearly every library will interact with unicode strings such as filesystem paths.

### SPDLog

SPDLog is a fast c++ logging library.

It is global because:

1. It is used by AFEX Core and users of AFEX Core.
2. Each application using AFEX Core is required to configure it's own global logging used throughout AFEX. I do not currently want to wrap the configuration properties of spdlog as abstracting those concepts is complicated and not-abstracting them creates a coupling to spdlog anyways.

### tomlplusplus

tomlplusplus is a TOML config file parser and serializer.
It is global because:

1. It is used by AFEX Core and users of AFEX Core.
2. AFEX Core is configured 