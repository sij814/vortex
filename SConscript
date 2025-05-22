import os

Import('env')

if not os.path.exists(Dir('.').srcnode().abspath):
  print("Not in vortex")
  env['HAVE_VORTEX'] = False
  Return()

env['HAVE_VORTEX'] = True
vortex_path = os.path.join(Dir('#').abspath, 'ext/vortex/')
print(vortex_path)
env.Prepend(CPPPATH=Dir('.').srcnode())
env.Append(
  LIBS=['simx', 'vortex-simx'],
  LIBPATH=[vortex_path+'build/runtime'],
  RPATH=[vortex_path+'build/runtime'],
  CPPPATH=[
  vortex_path+'sim/common', 
  vortex_path+'sim/simx',
  vortex_path+'sim/opaesimx',
  vortex_path+'build/hw/'
])

# export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(pwd)/ext/vortex/build/runtime"
# env.Append(LINKFLAGS=['-Wl,-rpath,/ext/vortex/build/runtime'])