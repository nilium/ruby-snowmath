#! /usr/bin/env ruby -w
# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'mkmf'

# Compile as C99
$CFLAGS += " -std=c99"

if ARGV.include?("--debug") || ARGV.include?("-D")
  $CFLAGS += " -g"
  puts "Building extension source in debug mode"
else
  $CFLAGS += " -O3 -fno-fast-math -fno-strict-aliasing"
  puts "Building extension source in release mode"
end

if ARGV.include?("--use-float") || ARGV.include?("-F")
  $CFLAGS += " -DUSE_FLOAT"
  puts "Using float as base type"
else
  puts "Using double as base type"
end

have_library('m', 'cos')

create_makefile('snow-math/bindings', 'snow-math/')
