#include "FSTReader.h"
#include <QSet>

Q_DECLARE_LOGGING_CATEGORY(fst_reader_json_logging)


/**
 * @brief Reads the new JSON-based FST format
 *
 */
class FSTJsonReader : public FSTReader {
public:
    /// Reads an FST mapping from the supplied data.
    virtual Mapping readMapping(const QByteArray& data) override;

    /// Writes an FST mapping to a byte array.
    virtual QByteArray writeMapping(const Mapping& mapping) override;

private:
    static QSet<QString> _jsonFields;
    void variantToJSon(const QVariant &variant, QJsonObject &);
};
