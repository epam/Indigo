import os
import time
import unittest

import requests

if __name__ == "__main__":
    service_url = "http://front/v2"
    if (
        "INDIGO_SERVICE_URL" in os.environ
        and len(os.environ["INDIGO_SERVICE_URL"]) > 0
    ):
        service_url = os.environ["INDIGO_SERVICE_URL"]
    start_time = time.time()
    service_is_up = False
    while time.time() - start_time < 60:
        try:
            if (
                requests.get(
                    "{}/info".format(service_url), timeout=None
                ).status_code
                == 200
            ):
                service_is_up = True
                break
            print("Waiting for front container getting ready...")
        except Exception:
            pass
        finally:
            time.sleep(1)
    if not service_is_up:
        raise RuntimeError(
            "Front container service seems to be down, stopping..."
        )

    print("Front container is ready, starting tests...")

    def load_tests(loader, tests, pattern):
        suite = unittest.TestSuite()
        ignore_pattern = ""
        if (
            "IGNORE_PATTERN" in os.environ
            and len(os.environ["IGNORE_PATTERN"]) > 0
        ):
            ignore_pattern = os.environ["IGNORE_PATTERN"]
        for all_test_suite in unittest.defaultTestLoader.discover(
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "api"),
            pattern="*.py",
        ):
            for test_suite in all_test_suite:
                if not (
                    len(ignore_pattern) > 0
                    and ignore_pattern in str(test_suite)
                ):
                    suite.addTests(test_suite)
        return suite

    unittest.main(verbosity=2, warnings="ignore")
