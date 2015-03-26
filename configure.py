#!/usr/bin/env python

"""Ninja build configurator for window library"""

import sys
import os

sys.path.insert( 0, os.path.join( 'build', 'ninja' ) )

import generator

dependlibs = [ 'foundation' ]

generator = generator.Generator( project = 'window', dependlibs = dependlibs, variables = [ ( 'bundleidentifier', 'com.rampantpixels.window.$(binname)' ) ] )
target = generator.target
writer = generator.writer
toolchain = generator.toolchain

window_lib = generator.lib( module = 'window', sources = [
  'event.c', 'version.c', 'window.c', 'window_android.c', 'window_ios.m', 'window_linux.c', 'window_osx.m', 'window_windows.c' ] )

includepaths = generator.test_includepaths()

gllibs = []
glframeworks = []
if target.is_macosx():
  glframeworks = [ 'OpenGL' ]
elif target.is_ios():
  glframeworks = [ 'QuartzCore', 'OpenGLES' ]

test_cases = [
  'window'
]
if target.is_ios() or target.is_android():
  #Build one fat binary with all test cases
  test_resources = []
  test_extrasources = []
  test_cases += [ 'all' ]
  if target.is_ios():
    test_resources = [ os.path.join( 'all', 'ios', item ) for item in [ 'test-all.plist', 'Images.xcassets', 'test-all.xib' ] ]
  elif target.is_android():
    test_resources = [ os.path.join( 'all', 'android', item ) for item in [
      'AndroidManifest.xml', os.path.join( 'layout', 'main.xml' ), os.path.join( 'values', 'strings.xml' ),
      os.path.join( 'drawable-ldpi', 'icon.png' ), os.path.join( 'drawable-mdpi', 'icon.png' ), os.path.join( 'drawable-hdpi', 'icon.png' ),
      os.path.join( 'drawable-xhdpi', 'icon.png' ), os.path.join( 'drawable-xxhdpi', 'icon.png' ), os.path.join( 'drawable-xxxhdpi', 'icon.png' )
    ] ]
  generator.app( module = '', sources = [ os.path.join( module, 'main.c' ) for module in test_cases ] + test_extrasources, binname = 'test-all', basepath = 'test', implicit_deps = [ window_lib ], libs = [ 'test', 'window', 'foundation' ], resources = test_resources, includepaths = includepaths, extralibs = gllibs, extraframeworks = glframeworks )
else:
  #Build one binary per test case
  generator.bin( module = 'all', sources = [ 'main.c' ], binname = 'test-all', basepath = 'test', implicit_deps = [ window_lib ], libs = [ 'window' ], includepaths = includepaths )
  for test in test_cases:
    if target.is_macosx():
      test_resources = [ os.path.join( 'osx', item ) for item in [ 'test-' + test + '.plist', 'Images.xcassets', 'test-' + test + '.xib' ] ]
      generator.app( module = test, sources = [ 'main.c' ], binname = 'test-' + test, basepath = 'test', implicit_deps = [ window_lib ], libs = [ 'test', 'window' ], resources = test_resources, includepaths = includepaths, extralibs = gllibs, extraframeworks = glframeworks )
    else:
      generator.bin( module = test, sources = [ 'main.c' ], binname = 'test-' + test, basepath = 'test', implicit_deps = [ window_lib ], libs = [ 'test', 'window' ], includepaths = includepaths )
