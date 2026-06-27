#!/bin/bash

for i in {1..3}
do
    curl -s \
        -H "User-Agent: LoadBalancerTest" \
        -H "X-Request-ID: $i" \
        localhost:8080
    echo
done

# chmod +x script.sh && ./test.sh
# bash test.sh