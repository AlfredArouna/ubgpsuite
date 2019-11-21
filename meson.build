project('ubgpsuite', 'c', version : '0.1',
                           meson_version : '>=0.48.0',
                           license : ['LGPLv3+', 'GPLv3+'],
                           default_options : [ 'c_std=c11',
                                               'warning_level=3',
                                               'b_ndebug=if-release' ])

cc = meson.get_compiler('c')

add_project_arguments(
    '-D_POSIX_C_SOURCE=200809L',
    '-D_XOPEN_SOURCE=500',
    language : 'c'
)

if cc.has_argument('-fvisibility=hidden')
    add_project_arguments('-fvisibility=hidden', language: 'c')
endif
if cc.has_argument('-Wno-missing-field-initializers')
    # We know missing fields are zeroed out...
    add_project_arguments('-Wno-missing-field-initializers', language: 'c')
endif

ubgp_args = []

zlib_dep = dependency('zlib')
bz2_dep  = cc.find_library('bz2', required : true)
lzma_dep = dependency('liblzma', version: '>=5.1.1', required : get_option('enable-lzma'))
if lzma_dep.found()
    ubgp_args += '-DUBGP_IO_XZ'
endif

lz4_dep = dependency('liblz4', required : get_option('enable-lz4'))
if lz4_dep.found()
    ubgp_args += '-DUBGP_IO_LZ4'
endif

if get_option('use-c-u128')
    ubgp_args += '-DUBGP_C_U128'
endif

ubgp = library('ubgp',
    sources : [
        'src/ubgp/bgpattribs.c',
        'src/ubgp/bgp.c',
        'src/ubgp/bgpparams.c',
        'src/ubgp/dumppacket.c',
        'src/ubgp/filterdump.c',
        'src/ubgp/filterintrin.c',
        'src/ubgp/filterpacket.c',
        'src/ubgp/hexdump.c',
        'src/ubgp/io.c',
        'src/ubgp/mrt.c',
        'src/ubgp/netaddr.c',
        'src/ubgp/patriciatrie.c',
        'src/ubgp/strutil.c',
        'src/ubgp/u128.c',
        'src/ubgp/vt100.c'
    ],
    c_args : ubgp_args,
    dependencies : [ zlib_dep, bz2_dep, lzma_dep, lz4_dep ],
    install : true
)
ubgp_dep = declare_dependency(compile_args : ubgp_args,
                              link_with : ubgp)

if get_option('build-tests')

    cunit_dep = dependency('cunit')

    bgp_test = executable('bgp_test',
        sources : [
            'src/test/bgp/attribs.c',
            'src/test/bgp/main.c',
            'src/test/bgp/open.c',
            'src/test/bgp/update.c'
        ],
        dependencies : [ ubgp_dep, cunit_dep ]
    )
    test('bgp', bgp_test)

    core_test = executable('core_test',
        sources : [
            'src/test/core/main.c',
            'src/test/core/dumppacket_t.c',
            'src/test/core/hexdump_t.c',
            'src/test/core/io_t.c',
            'src/test/core/netaddr_t.c',
            'src/test/core/patriciatrie_t.c',
            'src/test/core/strutil_t.c',
            'src/test/core/u128_t.c'
        ],
        dependencies : [ ubgp_dep, cunit_dep ]
    )
    test('core', core_test)
endif
if get_option('build-benchmarks')
    cbench_dep = dependency('cbench', static : true, fallback : [ 'cbench', 'cbench_dep' ])

        core_bench = executable('core_bench',
        sources : [
            'src/bench/core/main.c',
            'src/bench/core/strutil_b.c',
            'src/bench/core/patriciatrie_b.c',
            'src/bench/core/netaddr_b.c'
        ],
        dependencies : [ ubgp_dep, cbench_dep ]
    )
    benchmark('core', core_bench)

    bgp_bench = executable('bgp_bench',
        sources : [
            'src/bench/bgp/main.c',
            'src/bench/bgp/update_b.c'
        ],
        dependencies : [ ubgp_dep, cbench_dep ]
    )
    benchmark('bgp', bgp_bench)
endif

if get_option('build-docs')
    if find_program('hotdoc', required : get_option('build-docs')).found()
        subdir('docs')
    else
        message('Not building documentation as hotdoc was not found')
    endif
endif

if get_option('build-bgpgrep')

    bgpgrep = executable('bgpgrep',
        sources : [
            'src/bgpgrep/main.c',
            'src/bgpgrep/mrtdataread.c',
            'src/bgpgrep/progutil.c',
            'src/bgpgrep/parse.c'
        ],
        dependencies : [ ubgp_dep ],
        install : true
    )

    install_man('src/bgpgrep/bgpgrep.1')
endif
