#pragma once

#include <QLabel>

namespace JoltPhysics
{
    namespace Editor
    {
        class DocumentationLinkWidget
            : public QLabel
        {
        public:
            explicit DocumentationLinkWidget(const QString& linkFormat, const QString& linkAddress);
        };
    }
}
