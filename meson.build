project('libbinary-format', 'cpp', version: '1.0.0', default_options: ['cpp_std=c++17'])


# Project files
# =============

sources = [
    'src/data_reader.cpp',
]

includes = include_directories(
    'include',
)

headers = [
    'include/libbinary_format/data_reader.hpp',
    'include/libbinary_format/read_uint.hpp',
]


# Test files
# ==========

test_sources = [
    'test/src/test.cpp'
]

test_includes = [
    include_directories('test/include'),
    includes,
]


# Library
# =======

libbinary_format = library(
    'libbinary_format',
    sources,
    include_directories: includes,
    install: true,
)


# Test executable
# ===============

catch2_test_executable = executable(
    'catch2_test',
    test_sources,
    include_directories: test_includes,
    link_with: libbinary_format,
)


# Tests and installation
# ======================

test('catch2_test_suit', catch2_test_executable)
install_headers(headers, subdir: 'libbinary_format')

# Pkg-config
# ==========

pkg = import('pkgconfig')
pkg.generate(
    description: 'A small library helping in reading binary formats. Designed specifically for Pyke',
    filebase: 'libbinary_format',
    libraries: libbinary_format,
    name: 'libbinary_format',
    subdirs: 'include',
)
