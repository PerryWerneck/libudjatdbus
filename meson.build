# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

project(
	'udjatdbus', 
	['cpp'],
	version: '2.0.0',
	default_options : ['c_std=c11', 'cpp_std=c++17'],
	license: 'GPL-3.0-or-later',
)

project_description = 'DBus client/server for Udjat'

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
#subdir('po')

#
# Required libraries
#
libudjat = dependency('libudjat')
dbus = dependency('dbus-1')
lib_deps = [
  libudjat,
  dbus,
]

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
endif

compiler_flags = [
  '-ggdb3',
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-fPIC',
  '-DHAVE_CONFIG_H=1',
  '-DBUILD_DATE='+time_epoch,
]

if enable_debug
  compiler_flags += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags += [
	'-DNDEBUG=1'
  ]
endif

add_project_arguments(cxx.get_supported_arguments(compiler_flags), language: 'cpp')

#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)

if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  app_conf.set('HAVE_UNISTD_H', 1)
endif

includes_dir = include_directories('src/include')

#
# Sources
#
lib_src = [
  'src/library/alert.cc',
  'src/library/connection/abstract.cc',
  'src/library/connection/call.cc',
  'src/library/connection/named.cc',
  'src/library/connection/session.cc',
  'src/library/connection/starter.cc',
  'src/library/connection/system.cc',
  'src/library/connection/timeout.cc',
  'src/library/connection/user.cc',
  'src/library/connection/watch.cc',
  'src/library/interface.cc',
  'src/library/member.cc',
  'src/library/message/message.cc',
  'src/library/signal.cc',
  'src/library/value.cc',
]

module_src = [
  'src/module/init.cc',
]

test_src = [
  'src/testprogram/testprogram.cc'
]

#
# SDK
# https://mesonbuild.com/Pkgconfig-module.html
#
pkg = import('pkgconfig')

#
# SDK
#
pkg.generate(
  name: 'lib' + meson.project_name(),
  description: project_description,
  requires: [ 'dbus-1', 'libudjat' ],
  libraries: [ '-l' + meson.project_name() ]
)

pkg.generate(
  name: 'lib' + meson.project_name() + '-static',
  description: project_description,
  requires: [ 'dbus-1' ],
  libraries: [ '-l:lib' + meson.project_name() + '.a' ]
)

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]
dynamic = shared_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  version : libversion,
  soversion : libversion,
  gnu_symbol_visibility: 'hidden',
  dependencies: lib_deps,
  include_directories: includes_dir
)

static_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  dependencies: lib_deps,
  include_directories: includes_dir
)

# https://mesonbuild.com/Reference-manual_functions.html#shared_library
module = shared_library(
  'dbus', 
  config_src + module_src,
  name_prefix: 'udjat-module-',
  install: true,
  link_with: dynamic,
  install_dir: libudjat.get_variable('module_path'),
  gnu_symbol_visibility: 'hidden',
  dependencies: lib_deps,
  include_directories: includes_dir
)

executable(
  'dbustest',
  config_src + test_src,
  install: false,
  link_with : [ dynamic, module ],
  link_args: [ '-l:libudjat-testing.a' ],
  dependencies: [ libudjat, dbus ],
  include_directories: includes_dir
)

install_headers(
  'src/include/udjat/alert/d-bus.h',
  subdir: 'udjat/alert'  
)

install_headers(
  'src/include/udjat/tools/dbus.h',
  subdir: 'udjat/tools'  
)

install_headers(
  'src/include/udjat/tools/dbus/connection.h',
  'src/include/udjat/tools/dbus/defs.h',
  'src/include/udjat/tools/dbus/interface.h',
  'src/include/udjat/tools/dbus/member.h',
  'src/include/udjat/tools/dbus/message.h',
  'src/include/udjat/tools/dbus/signal.h',
  subdir: 'udjat/tools/dbus'  
)
