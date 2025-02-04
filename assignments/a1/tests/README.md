## Testing

Tested on Arch Linux and macOS.

The directory contains a google test suite and a stress test script.

- Run `make google` and then `make test` to run the google test suite.
- Run `make_dummy_users.sh` to add dummy users to the `users.txt` file.
- Run `client.sh` using xargs to run the stress test script (more details in the script).

Additionally, manually testing was done to cover all edge cases and ensure the correctness of the program.
