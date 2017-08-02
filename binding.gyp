{
  "targets": [
    {
      "target_name": "F22U",
      "sources": [ "src/F22U.cc", "src/Thread.cc" ],
      "include_dirs": [
        "tmp/include",
        "src"
      ],
      "link_settings": {
        "libraries": [
          "-lxt22Udll",
          "-lARTH_DLL"
        ],
        "library_dirs": [
          "tmp/lib"
        ]
      },
      "msbuild_settings": {
        "Link": {
          "ImageHasSafeExceptionHandlers": "false"
        }
      }
    }
  ]
}
