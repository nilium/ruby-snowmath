#! /usr/bin/env ruby -w
# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'mkmf'

# Compile as C99
$CFLAGS += " -std=c99 -march=native"

OptKVPair = Struct.new(:key, :value)

option_mappings = {
  '-F'              => OptKVPair[:build_float, true],
  '--use-float'     => OptKVPair[:build_float, true],
  '-NF'             => OptKVPair[:build_float, false],
  '--use-double'    => OptKVPair[:build_float, false],

  '-FM'             => OptKVPair[:build_fast_math, true],
  '--use-fast-math' => OptKVPair[:build_fast_math, true],
  '-NFM'            => OptKVPair[:build_fast_math, false],
  '--no-fast-math'  => OptKVPair[:build_fast_math, false],

  '-D'              => OptKVPair[:build_debug, true],
  '--debug'         => OptKVPair[:build_debug, true],
  '-ND'             => OptKVPair[:build_debug, false],
  '--release'       => OptKVPair[:build_debug, false]
}

options = {
  :build_float => false,
  :build_fast_math => false,
  :build_debug => false
}

ARGV.each {
  |arg|
  pair = option_mappings[arg]
  if pair
    options[pair.key] = pair.value
  else
    $stderr.puts "Unrecognized install option: #{arg}"
  end
}

if options[:build_debug]
  $CFLAGS += " -g"
  $stdout.puts "Building extension in debug mode"
else
  # mfpmath is ignored on clang, FYI
  $CFLAGS += " -O3 -fno-strict-aliasing -mfpmath=sse"
  $stdout.puts "Building extension in release mode"
end

if options[:build_fast_math] && !options[:build_debug]
  $stdout.puts "Building with -ffast-math enabled"
else
  $CFLAGS += " -fno-fast-math"
end

if options[:build_float]
  $CFLAGS += " -DUSE_FLOAT"
  $stdout.puts "Using float as base type"
else
  $stdout.puts "Using double as base type"
end

have_library('m', 'cos')

create_makefile('snow-math/bindings', 'snow-math/')
