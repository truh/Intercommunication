import os
env = Environment(ENV = os.environ)
env.Append(CPPPATH = ['.', 'include'])

SOURCES = Split("""
src/blink/blink.c
src/blink/startup_main.c
""")

# Program('hello',['src/hello.c','src/startup_main.c'])
env.Program('hello', SOURCES)

Repository(os.environ['HOME'] + '/opt/tivaware')

