#!python3.7
"""
Script to setup a venv
"""

import argparse
import os
import subprocess
import venv

class TestEnvBuilder(venv.EnvBuilder):
    """
    Replaces the BATCH script for creating a suitable venv for running the unit tests.
    Conditions for use:
     + A `requirements.txt` file must exist
     By default this is relative to the current run path, but this can be specified programmatically.
    """

    def __init__(self, *args, requirements=None, **kwargs):  #pylint: disable=line-too-long
        """
        :param requirements: Can be any path. If non, this step is skipped.
        """
        super().__init__(*args, **kwargs)
        self.requirements = requirements or ['requirements.txt']

    def post_setup(self, context):
        """
        Hook for setting up proper env for testing.
        """
        super().post_setup(context)

        print('Upgrading pip')
        self.pip_install(context, '--upgrade', 'pip', "setuptools", "wheel")
        for reqs in self.requirements:
            print(f'Installing requirements from {reqs}')
            self.pip_install(context, '-r', reqs)

    @staticmethod
    def pip_install(context, *args):
        """
        We run pip in isolated mode to avoid side effects from
        environment vars, the current directory and anything else
        intended for the global Python environment
        (same as EnvBuilder's _setup_pip)
        """
        ret = subprocess.call((context.env_exe, '-Im', 'pip', 'install') + args, stderr=subprocess.STDOUT)
        if ret:
            raise Exception(f'pip install command result was not 0 but {ret}')


if __name__ == '__main__':
    PARSER = argparse.ArgumentParser()
    PARSER.add_argument('-r', '--requirements', type=str, nargs='*')
    ARGS = PARSER.parse_args()

    CURRENTDIR = os.getcwd()
    TestEnvBuilder(clear=True, with_pip=True, requirements=ARGS.requirements).create('venv')
