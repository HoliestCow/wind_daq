import uuid
import ctypes
import hashlib


class thrift_uuid:
    def generate_thrift_uuid(self):
        generated_uuid = uuid.uuid4().int
        return self.convert_real_uuid_to_thrift_uuid(generated_uuid)

    def generate_thrift_uuid_with_seed(self, namespace, name):
        generated_uuid = uuid.uuid3(hashlib.md5(namespace).digest(), name.encode('utf-8')).int
        return self.convert_real_uuid_to_thrift_uuid(generated_uuid)

    def generate_thrift_uuid_with_name(self, name):
        generated_uuid = uuid.uuid3(uuid.NAMESPACE_DNS, name.encode('utf-8')).int
        return self.convert_real_uuid_to_thrift_uuid(generated_uuid)

    def convert_thrift_uuid_to_real_uuid(self, thrift_uuid):
        return (ctypes.c_uint64(thrift_uuid.mostSignificantBits).value << 64) | ctypes.c_uint64(thrift_uuid.leastSignificantBits).value

    def convert_real_uuid_to_thrift_uuid(self, uuid):
        return (ctypes.c_int64((uuid & 0xFFFFFFFFFFFFFFFF0000000000000000) >> 64).value,
                ctypes.c_int64(uuid & 0xFFFFFFFFFFFFFFFF).value)
