#!/bin/bash
alias=$(cat $1 | grep "alias :=")
alias=${alias:9}
prefix=$(cat $1 | grep "prefix :=")
prefix=${prefix:10}
revision=$(cat $1 | grep "revision :=")
revision=$(expr ${revision:12} + 1)
build=$(cat $1 | grep "build :=")
build=${build:9}
echo alias := $alias > $1
echo prefix := $prefix >> $1
echo date := $(date +%Y%m%d) >> $1
echo time := $(date +%H%M) >> $1
echo revision := $revision >> $1
echo build := $build >> $1
echo "sdk_version := \$(prefix)_\$(revision)\$(alias)_\$(date)_\$(time)" >> $1
echo "Created $1 with revision $revision"
