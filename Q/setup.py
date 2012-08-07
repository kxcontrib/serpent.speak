from distutils.core import setup, Extension, Distribution
from distutils.command.install_lib import install_lib
import os, sys
####

class QDistribution(Distribution):
    def finalize_options(self):
        self.cmdclass['install_lib'] = qinstall_lib
        self.qhome = os.getenv('QHOME') or os.path.join(os.getenv('HOME'), 'q')
        u = os.uname()
        if u[0] == 'Linux':
            o = 'l'
        elif u[0] == 'SunOS':
            o = 'v' if u[-1] == 'i86pc' else 's'
        else:
            sys.stderr.write("Unknown platform: %s\n" % str(u))
            sys.exit(1)
        self.qarch = o+('32', '64')[sys.maxint > 2147483647]
        self.install_data = os.path.join(self.qhome, self.qarch)
        
class QExtension(Extension):
    pass

        

class qinstall_lib(install_lib):
    def install(self):
        dist = self.distribution
        qdst = os.path.join(dist.qhome, dist.qarch)
        # TODO: eliminate hard-coded names
        pyfiles = ['_k.so', 'q.py', 'qc.py',]
        if os.path.isdir(self.build_dir):
            outfiles = [self.copy_file(os.path.join(self.build_dir, f),
                                       self.install_dir)[0] for f in pyfiles]
            outfiles += [self.copy_file(os.path.join(self.build_dir, 'py.so'), qdst)[0]]
            outfiles += [self.copy_file(os.path.join(self.build_dir, 'p.so'), qdst)[0]]
            outfiles += [self.copy_file('p.k', dist.qhome)[0]]
            outfiles += [self.copy_file('python.q', dist.qhome)[0]]
        else:
            self.warn("'%s' does not exist" % self.build_dir)
            return
        return outfiles


python_lib_dir =  os.path.join(sys.exec_prefix, 'lib')
module_k = Extension('_k',
                     sources=[ '_k.c',],
                     extra_compile_args = ['-g'],
                     include_dirs = [ ],)
modulepy = QExtension('py',
                      sources=[ 'py.c',],
                      extra_compile_args = ['-g'],
                      runtime_library_dirs = [python_lib_dir],
                      )

modulep = QExtension('p',
                      sources=[ 'p.c',],
                      extra_compile_args = ['-g'],
                      runtime_library_dirs = [python_lib_dir],
                      )

setup(distclass=QDistribution,
      name='pyq',
      version='3.0.1',
      url='http://code.kx.com/wiki/Contrib/PyQ',
      author='The Parseltongue Project',
      author_email='serpent.speak@gmail.com',
      ext_modules=[ module_k, modulepy, modulep],
      py_modules=['q','qc'],
      )
