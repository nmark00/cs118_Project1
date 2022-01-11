#!/bin/bash

set +e

PASS=$((0))
FAIL=$((0))

head -c 256 < /dev/urandom > test1.jpeg
head -c 384 < /dev/urandom > Test2.JPeG

curl -s -D head.txt http://localhost:8080/test1.jpeg -o file

echo -ne "Checking HTTP status code ... "
if cat head.txt | grep -i "HTTP/1\.." | grep -qi "200 OK" ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - no HTTP header found"
fi

echo -ne "Checking content length ... "
if cat head.txt | grep -i "Content-Length:" | grep -qi "256" ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - incorrect or missing content length"
fi

echo -ne "Checking if content is correct ... "
if cmp --silent test1.jpeg file ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - content not same"
fi

# ================================================================
echo -e "Checking case insensitivity"
curl -s -D head.txt http://localhost:8080/test2.jpeg -o file

echo -ne "Checking HTTP status code ... "
if cat head.txt | grep -i "HTTP/1\.." | grep -qi "200 OK" ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - no HTTP header found"
fi

echo -ne "Checking if content is correct ... "
if cmp --silent Test2.JPeG file ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - content not same"
fi

# ================================================================
echo -e "Checking GET without extension"
curl -s -D head.txt http://localhost:8080/Test2 -o file

echo -ne "Checking HTTP status code ... "
if cat head.txt | grep -i "HTTP/1\.." | grep -qi "200 OK" ; then
    PASS=$(($PASS+1))
    echo -e "pass"
else
    FAIL=$(($FAIL+1))
    echo -e "fail - no HTTP header found"
fi

rm file head.txt
echo -e ""
echo "Passed $PASS tests, $FAIL failed"