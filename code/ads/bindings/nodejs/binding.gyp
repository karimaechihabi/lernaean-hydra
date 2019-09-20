{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "jsads.cc" ],
      'include_dirs': [
        '../../include',
      ],
      'libraries': [
        '-lm','-lreadline', '-lads', '-L../../../lib/'
      ]
    }
  ]
}

