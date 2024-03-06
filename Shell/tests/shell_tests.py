#!/usr/bin/env python3

from unittest import TestCase

import unittest

import os.path
import sys
import subprocess
import random
import re

from shell_test_helpers import *

TOKENIZE = "./tokenize"
SHELL = "./shell"

class ShellTests(ShellTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(SHELL, *args, **kwargs)

    def test01(self):
        """ Shell prints the Welcome message and correct prompt """

        exe = subprocess.Popen(
                SHELL, 
                stdin = subprocess.DEVNULL, 
                stdout = subprocess.PIPE, 
                stderr = subprocess.STDOUT
              )

        try:
            (outb, _) = exe.communicate(timeout = 0.5)
        except subprocess.TimeoutExpired:
            exe.kill()
            (outb, _) = exe.communicate()

        out = try_decode(outb).strip()

        lines = out.splitlines()
        self.assertRegex(lines[0], "^Welcome to mini-shell.*")
        self.assertRegex(lines[1], "^shell \$.*")

    def test02(self):
        """ Exit command works """
        rc, actual = execute(SHELL, input = "exit\n")
        lines = actual.splitlines()
        matches = [re.match(".*Bye bye.", line) 
                   for line in lines[1:] 
                   if line.strip() != ""]

        self.assertTrue(
                any(matches),
                msg = "Could not find a Bye bye message")
 
    def test03(self):
        """ A single echo command works """
        output = self.run_shell("echo one")
        self.assertEqual(output, "one")

    def test04(self):
        """ Two echo commands work """
        output = self.run_shell("echo one\necho two")
        self.assertEqual(output, "one\ntwo")

    def test05(self):
        """ 'ls -a' works """
        actual = self.run_shell("ls -a")
        expected = sh("ls -a")
        self.assertEqual(actual, expected)

    def test06(self):
        """ Multiple file commands work """
        script = \
            "mkdir -p tmp\n"\
            "touch tmp/some_file\n"\
            "ls -1 tmp/\n"
        actual = self.run_shell(script)
        self.assertEqual(actual, "some_file")

    def test07(self):
        """ Multiple file commands work (without a newline at the end)"""
        script = \
            "mkdir -p tmp\n"\
            "touch tmp/some_file\n"\
            "ls -1 tmp/"
        actual = self.run_shell(script)
        self.assertEqual(actual, "some_file")

    def test08(self):
        """ Basic string tokens work """
        sh('rm -f "this is a file"')
        script = \
                'touch "this is a file"\n'\
                'ls "this is a file"'

        actual = self.run_shell(script)
        self.assertEqual(actual, "this is a file")

        sh('rm -f "this is a file"')

    def test09(self):
        """ Basic sequencing works """
        script = "echo one; echo two; echo three"
        actual = self.run_shell(script)
        self.assertEqual(actual, "one\ntwo\nthree")

    def test10(self):
        """ Simple redirection into a file """
        sh("rm -f nums.txt")
        script = 'echo -e "1\\n2\\n3\\n4\\n5" > nums.txt\n'\
                 'cat nums.txt'
        actual = self.run_shell(script)
        self.assertEqual(actual, "1\n2\n3\n4\n5")
        sh("rm -f nums.txt")

    def test11(self):
        """ Redirect into a file and read from the file in another command """
        sh("rm -f nums.txt")
        script = 'echo -e "1\\n2\\n3\\n4\\n5" > nums.txt\n'\
                 'sort -nr < nums.txt'
        actual = self.run_shell(script)
        self.assertEqual(actual, "5\n4\n3\n2\n1")
        sh("rm -f nums.txt")

    def test13(self):
        """ Test the 'prev' command """
        script = 'echo cs3650\n'\
                 'prev'
        actual = self.run_shell(script)
        self.assertEqual(actual, "cs3650\ncs3650")

    def test14(self):
        """ Check for the shuffle and sort combination """
        script = 'shuf -i 1-10 | sort -n'
        actual = self.run_shell(script).split("\n")
        expected = [str(i) for i in range(1, 11)]
        self.assertEqual(actual, expected)


if __name__ == '__main__':
    print(f"-= {YELLOW}Running tests for {SHELL}{RESET} =-")
    unittest.main(testRunner = unittest.TextTestRunner(resultclass = PrettierTextTestResult))


