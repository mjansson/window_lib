#!/usr/bin/env python

"""Ninja build generator"""

import argparse
import os
import pipes
import sys

import platform
import toolchain
import syntax

class Generator(object):
  def __init__( self, project, includepaths = [], dependlibs = [], libpaths = [], variables = None ):
    parser = argparse.ArgumentParser( description = 'Ninja build generator' )
    parser.add_argument( '-t', '--target',
                         help = 'Target platform',
                         choices = platform.supported_platforms() )
    parser.add_argument( '--host',
                         help = 'Host platform',
                         choices = platform.supported_platforms() )
    parser.add_argument( '--toolchain',
                         help = 'Toolchain to use',
                         choices = toolchain.supported_toolchains() )
    parser.add_argument( '-c', '--config', action = 'append',
                         help = 'Build configuration',
                         choices = ['debug', 'release', 'profile', 'deploy'],
                         default = [] )
    parser.add_argument( '-a', '--arch', action = 'append',
                         help = 'Add architecture',
                         choices = toolchain.supported_architectures(),
                         default = [] )
    parser.add_argument( '-i', '--includepath', action = 'append',
                         help = 'Add include path',
                         default = [] )
    parser.add_argument( '--monolithic', action='store_true',
                         help = 'Build monolithic test suite',
                         default = False )
    parser.add_argument( '--coverage', action='store_true',
                         help = 'Build with code coverage',
                         default = False )
    options = parser.parse_args()

    self.project = project
    self.target = platform.Platform(options.target)
    self.host = platform.Platform(options.host)
    archs = options.arch
    configs = options.config
    if configs is None or configs == []:
      configs = [ 'release' ]
    if includepaths is None:
      includepaths = []
    if not options.includepath is None:
      includepaths += options.includepath

    buildfile = open( 'build.ninja', 'w' )
    self.writer = syntax.Writer( buildfile )

    self.writer.variable( 'ninja_required_version', '1.3' )
    self.writer.newline()

    self.writer.comment( 'configure.py arguments' )
    self.writer.variable( 'configure_args', ' '.join( sys.argv[1:] ) )
    self.writer.newline()

    self.writer.comment( 'configure options' )
    self.writer.variable( 'configure_target', self.target.platform )
    self.writer.variable( 'configure_host', self.host.platform )

    env_keys = set( [ 'CC', 'AR', 'LINK', 'CFLAGS', 'ARFLAGS', 'LINKFLAGS' ] )
    configure_env = dict( ( key, os.environ[key] ) for key in os.environ if key in env_keys )
    if configure_env:
      config_str = ' '.join( [ key + '=' + pipes.quote( configure_env[key] ) for key in configure_env ] )
      writer.variable('configure_env', config_str + '$ ')

    if options.monolithic:
      if variables is None:
        variables = {}
      if isinstance( variables, dict ):
        variables['monolithic'] = True
      else:
        variables += [ ( 'monolithic', True ) ]
    if options.coverage:
      if variables is None:
        variables = {}
      if isinstance( variables, dict ):
        variables['coverage'] = True
      else:
        variables += [ ( 'coverage', True ) ]

    self.toolchain = toolchain.Toolchain( project, options.toolchain, self.host, self.target,
                                          archs, configs, includepaths, dependlibs, libpaths, variables,
                                          configure_env.get( 'CC' ),
                                          configure_env.get( 'AR' ),
                                          configure_env.get( 'LINK' ),
                                          configure_env.get( 'CFLAGS' ),
                                          configure_env.get( 'ARFLAGS' ),
                                          configure_env.get( 'LINKFLAGS' ) )

    self.writer.variable( 'configure_toolchain', self.toolchain.toolchain )
    self.writer.variable( 'configure_archs', self.toolchain.archs )
    self.writer.variable( 'configure_configs', self.toolchain.configs )
    self.writer.variable( 'configure_includepaths', self.toolchain.includepaths )
    self.writer.newline()

    self.toolchain.write_variables( self.writer )
    self.toolchain.write_rules( self.writer )

  def target( self ):
    return self.target

  def host( self ):
    return self.host

  def toolchain( self ):
    return self.toolchain

  def writer( self ):
    return self.writer

  def lib( self, module, sources, basepath = None, configs = None, includepaths = None ):
    return self.toolchain.lib( self.writer, module, sources, basepath, configs, includepaths )

  def bin( self, module, sources, binname, basepath = None, implicit_deps = None, libs = None, resources = None, configs = None, includepaths = None, extralibs = None, extraframeworks = None ):
    return self.toolchain.bin( self.writer, module, sources, binname, basepath, implicit_deps, libs, resources, configs, includepaths, extralibs, extraframeworks )

  def app( self, module, sources, binname, basepath = None, implicit_deps = None, libs = None, resources = None, configs = None, includepaths = None, extralibs = None, extraframeworks = None ):
    return self.toolchain.app( self.writer, module, sources, binname, basepath, implicit_deps, libs, resources, configs, includepaths, extralibs, extraframeworks )

  def test_includepaths( self ):
    if self.project == "foundation":
      return [ 'test' ]
    return [ 'test', os.path.join( '..', 'foundation_lib', 'test' ) ]

  def test_monolithic( self ):
    return self.toolchain.is_monolithic()
