
#include "FSTJsonReader.h"

Q_LOGGING_CATEGORY(fst_reader_json_logging, "overte.model-serializers.fst.json")


FSTReader::Mapping FSTJsonReader::readMapping(const QByteArray& data) {
    return Mapping();
};

    
QByteArray FSTJsonReader::writeMapping(const FSTReader::Mapping& mapping) {
    return QByteArray();
};
