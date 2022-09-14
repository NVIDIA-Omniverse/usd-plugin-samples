import os

def pytest_generate_tests(metafunc):
    """
    Invoked once per test, allows parameterization of tests based on options.
    """
    config = "debug"
    if os.getenv('CONFIG') != None:
        config = os.getenv('CONFIG')

    if hasattr(metafunc.cls, 'config'):
        metafunc.cls.config = config