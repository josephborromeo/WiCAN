# Tests

### Purpose
The purpose of this directory is to create a location where tests can be stored and executed from. 
- Initial idea is to contain speed tests that can be run to evaluate the performance of code changes.
- Secondonary idea is to have some regression like tests that will ensure proper parsing is done to ensure no messages are missed.

Results should be stored in some file (excel, csv, etc) to keep a history of all runs to ensure no backend changes severly affect performance.

### Tests
#### Speed Tests
Decoding: Decode Log file(s) as fast as possible while counting all received messages, and possible the occurances of each message?

Python Script will run all tests and spit out the results of each one for easy comparison

<br>Permutation of this test will include:
- Python
- Rust
- Python w/ Rust backend