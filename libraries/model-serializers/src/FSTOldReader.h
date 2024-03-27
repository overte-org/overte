#include "FSTReader.h"

Q_DECLARE_LOGGING_CATEGORY(fst_reader_old_logging)


/**
 * @brief Reads the old key/value pair based FST format
 * 
 */
class FSTOldReader: public FSTReader {
public:

    /// Reads an FST mapping from the supplied data.
    virtual Mapping readMapping(const QByteArray& data) override;

    /// Writes an FST mapping to a byte array.
    virtual QByteArray writeMapping(const Mapping& mapping) override;


private:
    void writeVariant(QBuffer& buffer, QVariantHash::const_iterator& it);
    Mapping parseMapping(QIODevice* device);
};