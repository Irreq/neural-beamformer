#!/bin/sh

# Run the unit tests using unittest
python3 -m unittest discover -s $(pwd) -vv

# Check the exit code of the unittest command
if [ $? -eq 0 ]; then
  # If the exit code is 0 (success), display "All tests passed" in green
  echo -e "\n\e[32m   ALL TESTS PASSED\e[0m\n"
else
  # If the exit code is not 0 (failure), display "Tests failed" in red
  echo -e "\n\e[31m   TESTS FAILED\e[0m\n"
fi
