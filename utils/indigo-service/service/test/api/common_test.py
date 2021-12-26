import json
import os
import unittest

import requests


class CommonTestCase(unittest.TestCase):
    """
    Test cases for common api
    """

    def setUp(self):
        service_url = "http://front/v2"
        if (
            "INDIGO_SERVICE_URL" in os.environ
            and len(os.environ["INDIGO_SERVICE_URL"]) > 0
        ):
            service_url = os.environ["INDIGO_SERVICE_URL"]
        self.url_prefix = "{}".format(service_url)

    def tearDown(self):
        pass

    @staticmethod
    def get_headers(d):
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json",
        }
        data = json.dumps(d)
        headers["Content-Length"] = len(data)
        return headers, data

    def test_info(self):
        headers, data = self.get_headers({})
        result = requests.get(
            self.url_prefix + "/info", headers=headers, data=data
        )
        self.assertEqual(200, result.status_code)
        result_data = json.loads(result.text)
        self.assertTrue(
            "indigo_version" in result_data and result_data["indigo_version"]
        )
        self.assertTrue(
            "service_version" in result_data and result_data["service_version"]
        )
        self.assertTrue(
            "imago_versions" in result_data and result_data["imago_versions"]
        )
