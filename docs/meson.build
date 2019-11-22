hotdoc = import('hotdoc')

documentation = hotdoc.generate_doc(meson.project_name(),
    project_version: meson.project_version(),
    sitemap: 'sitemap.txt',
    build_by_default: true,
    index: 'markdown/index.md',
    install: false,
    extra_assets: ['images'],
    include_paths: ['markdown'],
    languages: ['c'],
    c_source_roots: [ join_paths(meson.source_root(), 'src') ],
    c_sources: [
        join_paths(meson.source_root(), 'src/ubgp/*.h')
    ],

    default_license: 'CC-BY-SAv4.0',
    default_code_samples_license: 'CC0-1.0',
    devhelp_activate: true,
    syntax_highlighting_activate: true,
    c_smart_index: true
)
