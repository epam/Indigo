from indigo import IndigoObject


class IndigoRecord:

    def __init__(self, ind_obj: IndigoObject):
        self.fp = ind_obj.fingerprint("sim").oneBitList().split(" ")