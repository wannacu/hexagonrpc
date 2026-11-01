valgrind = find_program('valgrind', required : false)

if valgrind.found()
  add_test_setup('valgrind', exe_wrapper : valgrind)
endif

test_iobuffer = executable('test_iobuffer',
  'test_iobuffer.c',
  '../hexagonrpcd/iobuffer.c',
  c_args : cflags,
  include_directories : include,
)

sample_file = custom_target('sample_file',
  input : 'sample_file.txt',
  output : 'sample_file.txt',
  command : ['cp', '@INPUT@', '@OUTPUT@']
)
test_hexagonfs = executable('test_hexagonfs',
  'test_hexagonfs.c',
  '../hexagonrpcd/hexagonfs.c',
  '../hexagonrpcd/hexagonfs_mapped.c',
  c_args : cflags,
  include_directories : include,
)

test_hexagonrpc = executable('test_hexagonrpc',
  'test_hexagonrpc.c',
  '../libhexagonrpc/hexagonrpc.c',
  c_args : cflags,
  include_directories : include,
)

test('iobuffer', test_iobuffer)
test('hexagonfs', test_hexagonfs, args : [sample_file])
test('hexagonrpc', test_hexagonrpc)

test_sensor_registry = executable('test_sensor_registry',
  'test_sensor_registry.c',
  '../hexagonrpcd/hexagonfs.c',
  '../hexagonrpcd/hexagonfs_mapped.c',
  c_args : cflags,
  include_directories : include,
)
test('sensor-registry', test_sensor_registry)

test_listener_errors = executable('test_listener_errors',
  'test_listener_errors.c',
  '../hexagonrpcd/iobuffer.c',
  '../hexagonrpcd/interface/adsp_listener.c',
  c_args : cflags,
  include_directories : include,
  link_with : libhexagonrpc,
)
test('listener-errors', test_listener_errors)

test_listener_large_input = executable('test_listener_large_input',
  'test_listener_large_input.c',
  '../hexagonrpcd/iobuffer.c',
  '../hexagonrpcd/interface/adsp_listener.c',
  c_args : cflags,
  include_directories : include,
  link_with : libhexagonrpc,
)
test('listener-large-input', test_listener_large_input)
