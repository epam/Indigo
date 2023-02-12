#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# from app import app, is_indigo_db

import json
import os
import re
import unittest
from time import sleep

import requests


# @unittest.skip("Skip libraries test case")
class ImagoTestCase(unittest.TestCase):
    # Image files
    test_images = [
        ("imago_test_1.bmp", "image/bmp"),
        ("imago_test_1.dib", "image/bmp"),
        ("imago_test_1.jpg", "image/jpeg"),
        ("imago_test_1.pbm", "image/x-portable-bitmap"),
        ("imago_test_1.png", "image/png"),
        ("imago_test_1.tiff", "image/tiff"),
    ]

    def setUp(self):
        """
        Setting up host location for test
        :return:
        """
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
        """
        Setting headers for POST requests
        :param d: dictionary, which will be transformed into JSON for request
        :return: headers and data for POST requests
        """
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json",
        }
        data = json.dumps(d)
        headers["Content-Length"] = len(data)
        return headers, data

    def do_upload(
        self, file_path, content_type, version=None, settings=None, action=None
    ):
        """
        Upload file to library
        :param file_path: path to image file
        :param content_type: type of image to pass in request headers
        :param version: version of Imago. Default meaning equals to None
        :param settings: settings for Imago passed as a JSON. Default meaning eqauls to None
        :param action: determines logic of POST request in imago api
        :return: Return processed image in dictionary with mol file as a string
        """
        params = {}
        if version:
            params["version"] = version
        if settings:
            params["settings"] = settings
        if action:
            params["action"] = action
        with open(file_path, "rb") as f:
            result = requests.post(
                self.url_prefix + "/imago/uploads",
                headers={"Content-Type": content_type},
                params=params,
                data=f,
            )
        self.assertEqual(200, result.status_code)
        data = json.loads(result.text)
        upload_id = data["upload_id"]
        # Test LibraryUploadStatus
        state = "PENDING"
        while state != "SUCCESS":
            result = requests.get(
                self.url_prefix + "/imago/uploads/{0}".format(upload_id)
            )
            data = json.loads(result.text)
            state = data["state"]
            sleep(1)
        return data

    def imago_upload(self, filename, mime_type):
        """
        Pass images to do_upload and assert results from requests
        :param filename: name of image
        :param mime_type: type of image in request
        :return:
        """
        data = self.do_upload(
            os.path.join(
                os.path.dirname(os.path.realpath(__file__)),
                "..",
                "data",
                "imago",
                filename,
            ),
            mime_type,
        )
        self.assertEqual("SUCCESS", data["state"])
        self.assertTrue(
            """ 27 27  0  0  0  0  0  0  0  0999 V2000
    3.5130   -0.8924    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    8.3631   -1.3539    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    2.5062   -1.7179    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    4.4395   -1.7185    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    3.4917   -4.2144    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    5.4242   -4.8122    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.6370   -3.4549    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   11.1853   -3.6985    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    1.3503   -4.5920    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    2.4749   -4.8905    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    4.3458   -4.7818    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
    6.9007   -6.1959    0.0000 H   0  0  0  0  0  0  0  0  0  0  0  0
   11.6405   -6.4687    0.0000 Br  0  0  0  0  0  0  0  0  0  0  0  0
    9.0858   -7.0584    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3304   -1.0094    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    9.0670   -1.7364    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.4541   -1.7458    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.4917   -3.3978    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0576   -3.4072    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.7248   -4.1768    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3905   -4.2144    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.0743   -4.1487    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    4.8526   -4.1675    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.7248   -5.7349    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   10.3905   -5.7068    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0389   -6.4952    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.2658   -3.3750    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
 15 16  2  0  0  0  0
  2 16  1  0  0  0  0
  1 17  1  0  0  0  0
  3 17  1  0  0  0  0
  4 17  1  0  0  0  0
 17 18  1  0  0  0  0
  5 18  1  0  0  0  0
 16 19  1  0  0  0  0
 19 20  2  0  0  0  0
 19 21  1  0  0  0  0
  8 21  1  0  0  0  0
 18 22  1  0  0  0  0
  7 22  1  0  0  0  0
  9 22  1  0  0  0  0
 10 22  1  0  0  0  0
 18 23  1  0  0  0  0
  6 23  1  0  0  0  0
 11 23  1  0  0  0  0
 20 24  1  0  0  0  0
 12 24  1  0  0  0  0
 21 25  2  0  0  0  0
 13 25  1  0  0  0  0
 24 26  2  0  0  0  0
 25 26  1  0  0  0  0
 14 26  1  0  0  0  0
 20 27  1  0  0  0  0
 23 27  1  0  0  0  0
M  END
""",
            "\n".join(str(data["metadata"]["mol_str"]).splitlines()[3:]),
        )

    def test_imago_upload(self):
        """
        Pass images for Imago for testing
        :return:
        """
        for file_mime in self.test_images:
            self.imago_upload(file_mime[0], file_mime[1])

    def test_versions(self):
        """
        Check if available versions of Imago can process image
        :return:
        Assert is mol files returned by different versions
        """
        filename, mime_type = self.test_images[5]
        request = requests.get(self.url_prefix + "/info")
        versions = json.loads(request.text)["imago_versions"]
        for version in versions:
            data = self.do_upload(
                os.path.join(
                    os.path.dirname(os.path.realpath(__file__)),
                    "..",
                    "data",
                    "imago",
                    filename,
                ),
                mime_type,
                version=version,
            )
            self.assertEqual("SUCCESS", data["state"])
            self.assertTrue("mol_str" in data["metadata"])

    def test_settings(self):
        """
        Test POST request with configuration parameters for IMAGO in URI
        :return:
        Assert if imago processed image with passed configuration
        """
        filename, mime_type = self.test_images[5]
        txt = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "..",
            "data",
            "test_config_imago.inc",
        )
        settings = {}
        with open(txt, "r") as f:
            for line in f.readlines():
                param, mean = re.search("(.*) = (.*);", line).group(1, 2)
                settings[param] = mean
        data = self.do_upload(
            os.path.join(
                os.path.dirname(os.path.realpath(__file__)),
                "..",
                "data",
                "imago",
                filename,
            ),
            mime_type,
            settings=json.dumps(settings),
        )
        self.assertEqual("SUCCESS", data["state"])
        self.assertTrue("mol_str" in data["metadata"])

    def test_expire_wait_request(self):
        """
        Test expiration of second POST request by sending 5 second expire time limit
        :return:
        Assert if error message 410 were returned
        """
        filename, mime_type = self.test_images[5]
        path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "..",
            "data",
            "imago",
            filename,
        )
        with open(path, "rb") as f:
            result = requests.post(
                self.url_prefix + "/imago/uploads",
                headers={"Content-Type": mime_type},
                params={"action": "wait", "expires": 5},
                data=f,
            )
        sleep(6)
        data = json.loads(result.text)
        upload_id = data["upload_id"]
        result = requests.post(
            self.url_prefix + "/imago/uploads/{0}".format(upload_id),
            params={"action": "run"},
        )
        self.assertEqual(410, result.status_code)
        data = json.loads(result.text)
        self.assertEqual(
            data["error"], "Image are not available because of time limit"
        )

    def test_wait_request(self):
        """
        Test for second post request with default Imago settings
        :return:
        Assert if image was processed by Imago
        """
        filename, mime_type = self.test_images[5]
        path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "..",
            "data",
            "imago",
            filename,
        )
        txt = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "..",
            "data",
            "test_config_imago.inc",
        )
        settings = {}

        with open(txt, "r") as f:
            for line in f.readlines():
                param, mean = re.search("(.*) = (.*);", line).group(1, 2)
                settings[param] = mean
        with open(path, "rb") as f:
            result = requests.post(
                self.url_prefix + "/imago/uploads",
                headers={"Content-Type": mime_type},
                params={"action": "wait"},
                data=f,
            )
        self.assertEqual(200, result.status_code)
        data = json.loads(result.text)
        upload_id = data["upload_id"]
        result = requests.post(
            self.url_prefix + "/imago/uploads/{0}".format(upload_id),
            params={"action": "run"},
            data={"settings": json.dumps(settings)},
        )
        data = json.loads(result.text)
        self.assertTrue("mol_str" in data)


if __name__ == "__main__":
    exit(unittest.main(verbosity=2, warnings="ignore"))
