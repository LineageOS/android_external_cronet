#  Copyright (C) 2022 The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import classes_collector
import unittest

class TestGenJarjar(unittest.TestCase):

    def setUp(self):
        # This allows printing the entire diff in the output when there's a mismatch otherwise
        # the diff will be truncated.
        self.maxDiff = None

    def test_classes_collection(self):
        args = classes_collector.parse_arguments([
            "test_classes_collector_java.jar",
            "--prefix", "jarjar.prefix",
            "--output", "test-output-rules.txt",
            "--prefix-excludes", "testdata/test-package-prefix-excludes.txt",
            "--excludes", "testdata/test-excludes.txt",
        ])

        self.assertCountEqual([
            '"android.net.ParentClass"',
            '"android.net.ParentClass$ChildClassA"',
            '"jarjar.prefix.android.jarjared.FooClass"'
        ], classes_collector.make_classes_list(args))


if __name__ == '__main__':
    # Need verbosity=2 for the test results parser to find results
    unittest.main(verbosity=2)