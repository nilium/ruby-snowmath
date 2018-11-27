# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require File.expand_path('../lib/snow-math/version.rb', __FILE__)

Gem::Specification.new { |s|
  s.name        = 'snow-math'
  s.version     = Snow::SNOW_MATH_VERSION
  s.date        = '2018-11-26'
  s.summary     = 'Snow Math Types'
  s.description = 'Math types built on the SnowPalm math code'
  s.authors     = [ 'Noel Raymond Cower' ]
  s.email       = 'ncower@gmail.com'
  s.files       = Dir.glob('lib/**/*.rb') +
                  Dir.glob('ext/**/*.{c,h,rb}') +
                  [ 'COPYING', 'README.md' ]
  s.extensions << 'ext/extconf.rb'
  s.homepage    = 'https://github.com/nilium/ruby-snowmath'
  s.license     = 'Simplified BSD'
  s.has_rdoc    = true
  s.extra_rdoc_files = [
      'ext/snow-math/snow-math.c',
      'README.md',
      'COPYING'
  ]
  s.rdoc_options << '--title' << 'snowmath -- 3D Math Types' <<
                    '--main' << 'README.md' <<
                    '--markup=markdown' <<
                    '--line-numbers'
  s.required_ruby_version = '>= 2.0.0'
}
