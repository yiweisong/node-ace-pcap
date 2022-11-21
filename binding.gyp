{
  'targets': [{
      'target_name': 'ace-pcap',
      'sources': [
          'src/binding.cc',
          'src/live_device_capture.cc',
          'src/utils.cc'
      ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': ['-fno-exceptions'],
      'cflags_cc!': ['-fno-exceptions'],
      'conditions':[
          ['OS=="win"',
            {
              'sources': [],
              'include_dirs': [
                  'deps/Include',
              ],
              'defines': ['WPCAP'],
              'conditions': [
                  ['target_arch=="ia32"', {
                      'link_settings': {
                          'libraries': ['<(PRODUCT_DIR)/../../deps/Lib/wpcap.lib'],
                      },
                  }, {
                      'link_settings': {
                          'libraries': ['<(PRODUCT_DIR)/../../deps/Lib/x64/wpcap.lib'],
                      },
                  }],
              ],
              'msvs_settings': {
                'VCCLCompilerTool': { 'ExceptionHandling': 1 },
              }
            }
          ],
          ['OS=="mac"',
            {
              'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'MACOSX_DEPLOYMENT_TARGET': '10.9',
                'OTHER_CFLAGS': [
                  '-arch x86_64',
                  '-arch arm64'
                ],
                'OTHER_LDFLAGS': [
                  '-framework CoreFoundation',
                  '-framework IOKit',
                  '-arch x86_64',
                  '-arch arm64'
                ]
              },
              'link_settings': {
                  'libraries': ['-lpcap'],
              }
            }
          ],
          ['OS!="win"',
            {
              # POSIX
              'link_settings': {
                  'libraries': ['-lpcap'],
              },
            }
          ]
      ]
  }]
}
