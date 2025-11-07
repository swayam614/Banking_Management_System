#!/bin/bash

echo "🔧 Building Banking Management System..."

for module in Admin Customer Employee Handler Loan Manager Utils
do
    echo "➡️ Compiling $module module..."
    cd $module/src || exit
    ./run.sh
    cd ../..
done

echo
echo "✅ All modules compiled successfully!"
