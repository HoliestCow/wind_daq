import uuid
import ctypes
import hashlib


class Thrift_UUID:

    @staticmethod
    def generate_thrift_uuid():
        generated_uuid = uuid.uuid4().int
        return Thrift_UUID.convert_real_uuid_to_thrift_uuid(generated_uuid)

    @staticmethod
    def generate_thrift_uuid_with_seed(namespace, name):
        generated_uuid = uuid.uuid3(hashlib.md5(namespace).digest(), name.encode('utf-8')).int
        return Thrift_UUID.convert_real_uuid_to_thrift_uuid(generated_uuid)

    @staticmethod
    def generate_thrift_uuid_with_name(name):
        generated_uuid = uuid.uuid3(uuid.NAMESPACE_DNS, name.encode('utf-8')).int
        return Thrift_UUID.convert_real_uuid_to_thrift_uuid(generated_uuid)

    @staticmethod
    def convert_thrift_uuid_to_real_uuid(thrift_uuid):
        return (ctypes.c_uint64(thrift_uuid.mostSignificantBits).value << 64) | ctypes.c_uint64(thrift_uuid.leastSignificantBits).value

    @staticmethod
    def convert_real_uuid_to_thrift_uuid(uuid):
        return (ctypes.c_int64((uuid & 0xFFFFFFFFFFFFFFFF0000000000000000) >> 64).value,
                ctypes.c_int64(uuid & 0xFFFFFFFFFFFFFFFF).value)
