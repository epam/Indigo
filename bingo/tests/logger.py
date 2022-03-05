import logging

logger = logging.getLogger("bingo-tests")
logger.setLevel(logging.DEBUG)

fh = logging.FileHandler("bingo-tests.log", "w")
fh.setLevel(logging.DEBUG)

ch = logging.StreamHandler()
ch.setLevel(logging.ERROR)

formatter = logging.Formatter("%(asctime)s: [%(levelname)s] - %(message)s")
fh.setFormatter(formatter)
ch.setFormatter(formatter)

logger.addHandler(fh)
logger.addHandler(ch)
