"""PyQ: Python/Q Integration

PyQ provides seamless integration of Python and Q code. It brings
Python and Q interpretors in the same process and allows code written
in either of the languages to operate on the same data. In PyQ, Python
and Q objects live in the same memory space and share the same data.
"""
###############################################################################
metadata = dict(
    name='pyq',
    version='3.1.0',
    url='http://code.kx.com/wiki/Contrib/PyQ',
    author='The Parseltongue Project',
    author_email='serpent.speak@gmail.com',
    license='http://code.kx.com/wiki/TermsAndConditions',
    platforms='Linux, Solaris',
)
###############################################################################
from distutils.core import Extension
import os, sys
python_lib_dir =  os.path.join(sys.exec_prefix, 'lib')
_k = Extension('_k',
               sources=[ '_k.c',],
               extra_compile_args = [],
               include_dirs = [ ],)
py = Extension('py',
               sources=[ 'py.c',],
               extra_compile_args = [],
               runtime_library_dirs = [python_lib_dir],
               )
p = Extension('p',
              sources=[ 'p.c',],
              extra_compile_args = [],
              runtime_library_dirs = [python_lib_dir],
              )
metadata.update(
    py_modules=['q','qc'],
    q_modules=['python'],
    k_modules=['p'],
    ext_modules=[ _k, ],
    qext_modules=[ py, p, ],
)
###############################################################################
from distutils.core import setup, Extension, Command
from distutils.core import Distribution as _Distribution
from distutils.command.install_lib import install_lib as _install_lib
from distutils.command.install import install as _install
from distutils.command.build import build as _build
from distutils.command.config import config as _config
from distutils.command.build_ext import build_ext as _build_ext
from distutils.util import get_platform
from distutils.sysconfig import (customize_compiler, get_python_version,
                                 get_config_var)
try:
   from distutils.command.build_py import build_py_2to3 \
        as build_py
except ImportError:
   from distutils.command.build_py import build_py
####

class config(_config):
    def run(self):
        self.check_lib('python' + sys.version[:3])

class build(_build):
    user_options = _build.user_options + [
                ('build-qlib=', None,
                 "build directory for q/k modules"),
                ('build-qext=', None,
                 "build directory for q extension modules"),
                ]
    user_options.sort()

    def initialize_options(self):
        _build.initialize_options(self)
        self.build_qlib = None
        self.build_qext = None

    def finalize_options(self):
        _build.finalize_options(self)
        plat_specifier = ".%s-%s" % (self.plat_name, sys.version[:3])
        if self.build_qlib is None:
            self.build_qlib = os.path.join(self.build_base,
                                           'qlib' + plat_specifier)
        qarch = self.distribution.qarch
        kxver = self.distribution.kxver
        if self.build_qext is None:
            self.build_qext = os.path.join(self.build_base,
                                           'qext.%s-%s' % (qarch, kxver))
        self.build_temp += '-kx_' + kxver

    def has_qk_modules(self):
        return self.distribution.has_qlib()

    def has_qext(self):
        return self.distribution.has_qext()
    
    sub_commands = _build.sub_commands + [
        ('build_qk', has_qk_modules),
        ('build_qext', has_qext),
        ]

class build_qk(Command):
    description = "build pure Q/K modules"

    user_options = [
        ('build-lib=', 'd', "build directory"),
        ('force', 'f', "forcibly build everything (ignore file timestamps)"),
        ]

    def initialize_options(self):
        self.build_lib = None


    def finalize_options(self):
        self.set_undefined_options('build',
                                   ('build_qlib', 'build_lib'),
                                   ('force', 'force'))
        # Get the distribution options that are aliases for build_qk
        # options -- list of Q/K modules.
        self.q_modules = self.distribution.q_modules
        self.k_modules = self.distribution.k_modules
        pyver = sys.version[0:3] + getattr(sys, 'abiflags', '')
        self.pyver_rule = (None, 'PYVER:',
                           'PYVER: "%s"\n' % pyver)
        qpath = self.distribution.qexecutable
        if self.distribution.kxver >= '2.3':
            self.qpath_rule = (0, '#!', "#! %s\n" % qpath)
        else:
            self.qpath_rule = (0, '#!', "")
        self.q_module_rules =  [self.qpath_rule, self.pyver_rule]
        soabi = get_config_var('SOABI')
        if soabi:
           self.q_module_rules.append(((None, 'PYSO:',
                                        'PYSO: `$"py.%s"\n' % soabi)))


    def run(self):
        self.mkpath(self.build_lib)
        for m in self.q_modules:
            infile = m + '.q'
            outfile = os.path.join(self.build_lib, infile)
            self.build_q_module(infile, outfile)
        for m in self.k_modules:
            infile = m + '.k'
            outfile = os.path.join(self.build_lib, infile)
            self.build_k_module(infile, outfile)

    def build_k_module(self, infile, outfile):
        self.build_module(infile, outfile,
                          [self.pyver_rule])

    def build_q_module(self, infile, outfile):
        self.build_module(infile, outfile, self.q_module_rules)
        # copy executable flags
        inmode = os.stat(infile).st_mode
        if inmode & 0o111:
            outmode = os.stat(outfile).st_mode
            os.chmod(outfile, inmode & 0o111 | outmode)

    def build_module(self, infile, outfile, rules):
        adjust = {}
        sentinel = object()
        with open(infile) as source:
            for lineno, line in enumerate(source):
                if not line.strip():
                    adjust[lineno] = sentinel
                    break
                for rlineno, start, adjusted in rules:
                    if (rlineno is None or
                        lineno == rlineno) and line.startswith(start):
                        adjust[lineno] = adjusted
        if adjust:
            with open(infile) as source: 
              with open(outfile, 'w') as target:
                for lineno, line in enumerate(source):
                    a = adjust.get(lineno)
                    if a is sentinel:
                        target.write("/ ^^^ generated by setup ^^^\n")
                        target.writelines(source)
                        break
                    if a is None:
                        target.write(line)
                    else:
                        target.write(a)
        else:
            self.copy_file(infile, outfile, preserve_mode=0)

class build_ext(_build_ext):
    def get_ext_filename(self, ext_name):
        filename = _build_ext.get_ext_filename(self, ext_name)
        so_ext = get_config_var('SO')
        kxver = self.distribution.kxver
        return filename[:-len(so_ext)] + kxver.replace('.', '_') + so_ext

class build_qext(_build_ext):
    description = "build Q extension modules"

    user_options = [
        ('build-lib=', 'd', "build directory"),
        ('force', 'f', "forcibly build everything (ignore file timestamps)"),
        ]

    def swig_sources(self, sources, ext):
        return sources

    def finalize_options(self):
        from distutils import sysconfig
        self.set_undefined_options('build',
                                   ('build_qext', 'build_lib'),
                                   ('build_temp', 'build_temp'),
                                   ('compiler', 'compiler'),
                                   ('debug', 'debug'),
                                   ('force', 'force'),
                                   ('plat_name', 'plat_name'),
                                   )
        self.extensions = self.distribution.qext_modules
        
        # TODO: Don't add python stuff to q extentions that don't need it
        # Make sure Python's include directories (for Python.h, pyconfig.h,
        # etc.) are in the include search path.
        py_include = sysconfig.get_python_inc()
        plat_py_include = sysconfig.get_python_inc(plat_specific=1)
        if self.include_dirs is None:
            self.include_dirs = self.distribution.include_dirs or []
        if isinstance(self.include_dirs, str):
            self.include_dirs = self.include_dirs.split(os.pathsep)

        # Put the Python "system" include dir at the end, so that
        # any local include dirs take precedence.
        self.include_dirs.append(py_include)
        if plat_py_include != py_include:
            self.include_dirs.append(plat_py_include)
        self.ensure_string_list('libraries')

        # Life is easier if we're not forever checking for None, so
        # simplify these options to empty lists if unset
        if self.libraries is None:
            self.libraries = []
        if self.library_dirs is None:
            self.library_dirs = []
        elif type(self.library_dirs) is StringType:
            self.library_dirs = string.split(self.library_dirs, os.pathsep)

        if self.rpath is None:
            self.rpath = []
        elif type(self.rpath) is StringType:
            self.rpath = string.split(self.rpath, os.pathsep)

        if self.define:
            defines = [dfn.split(':') for dfn in self.define.split(',')]
            self.define = [(dfn if len(dfn) == 2 else dfn + ['1'])
                           for dfn in defines]
        else:
            self.define = []
        if self.undef:
            self.undef = self.undef.split(',')

    def run(self):
        from distutils.ccompiler import new_compiler
        if not self.extensions:
            return
        # Setup the CCompiler object that we'll use to do all the
        # compiling and linking
        self.compiler = new_compiler(compiler=self.compiler,
                                     verbose=self.verbose,
                                     dry_run=self.dry_run,
                                     force=self.force)
        customize_compiler(self.compiler)
         # And make sure that any compile/link-related options (which might
        # come from the command-line or from the setup script) are set in
        # that CCompiler object -- that way, they automatically apply to
        # all compiling and linking done here.
        if self.include_dirs is not None:
            self.compiler.set_include_dirs(self.include_dirs)
        if self.define is not None:
            # 'define' option is a list of (name,value) tuples
            for (name, value) in self.define:
                self.compiler.define_macro(name, value)
        if self.undef is not None:
            for macro in self.undef:
                self.compiler.undefine_macro(macro)
        if self.libraries is not None:
            self.compiler.set_libraries(self.libraries)
        if self.library_dirs is not None:
            self.compiler.set_library_dirs(self.library_dirs)
        if self.rpath is not None:
            self.compiler.set_runtime_library_dirs(self.rpath)
        if self.link_objects is not None:
            self.compiler.set_link_objects(self.link_objects)

        # Now actually compile and link everything.
        self.build_extensions()
       


class install_qlib(_install_lib):
    
    description = "install Q/K modules"
 
    def run(self):
        self.mkpath(self.install_dir)
        outfiles = self.copy_tree(self.build_dir, self.install_dir)

        

    def finalize_options(self):
        self.set_undefined_options('install',
                                   ('build_qlib', 'build_dir'),
                                   ('install_qlib', 'install_dir'),
                                   ('force', 'force'),
                                   ('compile', 'compile'),
                                   ('optimize', 'optimize'),
                                   ('skip_build', 'skip_build'),
                                  )
        if self.install_dir is None:
            self.install_dir = self.distribution.qhome 

class install_qext(_install_lib):
    
    description = "install q extension modules"
 
    def run(self):
        self.mkpath(self.install_dir)
        outfiles = self.copy_tree(self.build_dir, self.install_dir)

        

    def finalize_options(self):
        self.set_undefined_options('install',
                                   ('build_qext', 'build_dir'),
                                   ('install_qext', 'install_dir'),
                                   ('force', 'force'),
                                   ('compile', 'compile'),
                                   ('optimize', 'optimize'),
                                   ('skip_build', 'skip_build'),
                                  )
        if self.install_dir is None:
            self.install_dir = self.distribution.qhome 


    
class install(_install):
    def has_qlib(self):
        return self.distribution.has_qlib()

    def has_qext(self):
        return self.distribution.has_qext()

    def initialize_options(self):
        self.install_qlib = None
        self.build_qlib = None
        self.install_qext = None
        self.build_qext = None
        _install.initialize_options(self)

    def finalize_options(self):
        _install.finalize_options(self)
        self.set_undefined_options('build',
                                   ('build_qlib', 'build_qlib'),
                                   ('build_qext', 'build_qext'),
                                  )
        dst = self.distribution
        if self.install_qlib == None:
            self.install_qlib = dst.qhome
        if self.install_qext == None:
            self.install_qext = os.path.join(dst.qhome, dst.qarch)


    user_options = _install.user_options +[
        ('install-qlib=', None,
         "installation directory for all Q/K module distributions"),
        ('install-qext=', None,
         "installation directory for Q extension module distributions"),
        ]

    sub_commands = _install.sub_commands + [
        ('install_qlib', has_qlib),
        ('install_qext', has_qext),
        ]

class Distribution(_Distribution):
    def __init__ (self, attrs=None):
        self.k_modules = None
        self.q_modules = None
        self.qext_modules = None
        _Distribution.__init__(self, attrs)
    
    def has_qlib(self):
        return self.k_modules or self.q_modules

    def has_qext(self):
        return bool(self.qext_modules)

    def get_kxver(self, qhome):
        """Determine version of q installed at qhome
        """
        qk = os.path.join(qhome, 'q.k')
        with open(qk) as source:
            for line in source:
                if line.startswith('k:'):
                    return line[2:5]
        return '2.2'

    def finalize_options(self):
        self.cmdclass['config'] = config

        self.cmdclass['build'] = build
        self.cmdclass['build_py'] = build_py
        self.cmdclass['build_qk'] = build_qk
        self.cmdclass['build_ext'] = build_ext
        self.cmdclass['build_qext'] = build_qext

        self.cmdclass['install'] = install
        self.cmdclass['install_qlib'] = install_qlib
        self.cmdclass['install_qext'] = install_qext

        self.qhome = os.getenv('QHOME') or os.path.join(os.getenv('HOME'), 'q')
        u = os.uname()
        if u[0] == 'Linux':
            o = 'l'
        elif u[0] == 'SunOS':
            o = 'v' if u[-1] == 'i86pc' else 's'
        else:
            sys.stderr.write("Unknown platform: %s\n" % str(u))
            sys.exit(1)
        bits = 8 * get_config_var('SIZEOF_VOID_P')
        self.qarch = "%s%d" % (o, bits)
        self.install_data = os.path.join(self.qhome, self.qarch)
        self.kxver = self.get_kxver(self.qhome)
        self.qexecutable = os.path.join(self.qhome, self.qarch, 'q')
        _Distribution.finalize_options(self)
        for ext in self.ext_modules + self.qext_modules:
            ext.define_macros.append(('KXVER', self.kxver[0]))
            ext.define_macros.append(('QVER', self.kxver.replace('.', '_')))
            if sys.hexversion >= 0x3000000:
               ext.define_macros.append(('PY3K', '1'))


###############################################################################
summary, details = __doc__.split('\n\n', 2)
download_url = ("http://code.kx.com/wsvn/code/contrib/"
                "serpent.speak/trunk/Q/dist/"
                "%(name)s-%(version)s.tar.gz?op=dl" % metadata) 
setup(distclass=Distribution,
      description=summary,
      long_description=details,
      download_url=download_url,
      **metadata)
