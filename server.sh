#!/bin/bash

echo
echo "🖥️ Starting the server..."
cd server-side

./run.sh

gcc server.c -I Admin/include -I Customer/include -I Employee/include -I Handler/include -I Loan/include -I Manager/include -I Utils/include -L Admin/lib -L Customer/lib -L Employee/lib -L Handler/lib -L Loan/lib -L Manager/lib -L Utils/lib  -ladmin -lcustomer -lemployee -lhandler -lloan -lmanager -lutils -o server
./server