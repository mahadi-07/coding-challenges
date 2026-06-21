gcc backend.c -o backend
gcc round_robin.c -o rr

./backend 8091 backend-A
./backend 8092 backend-B
./backend 8093 backend-C

./rr