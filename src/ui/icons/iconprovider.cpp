#include "iconprovider.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QGuiApplication>

IconProvider::IconProvider()
{
}

QIcon IconProvider::icon(IconId id, const QColor& color)
{
    if (m_cache.contains(id, color))
    {
        return m_cache.get(id, color);
    }

    QIcon icon = recolorIcon(iconPath(id), color);

    m_cache.put(id, color, icon);

    return icon;
}

void IconProvider::clearCache()
{
    m_cache.clear();
}

QPixmap IconProvider::renderSvgColored(
    const QString& path,
    const QSize& size,
    const QColor& color) const
{
    // Investigating weird cutting off of icons on Windows
    // with 125% and higher scaling.
    // Disabling DPR multiplier temporarly.
    //
    // const qreal dpr = qApp->devicePixelRatio();

    QSvgRenderer renderer(path);

    // QPixmap pix(size * dpr);
    QPixmap pix(size);

    // pix.setDevicePixelRatio(dpr);

    pix.fill(Qt::transparent);

    QPainter painter(&pix);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    renderer.render(&painter);

    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pix.rect(), color);

    return pix;
}

QIcon IconProvider::recolorIcon(const QString& path, const QColor& color) const
{
    QIcon icon;

    icon.addPixmap(renderSvgColored(
        path,
        QSize(128, 128),
        color));

    return icon;
}

QString IconProvider::iconPath(IconId id)
{
    switch (id)
    {
    case IconId::NewVault:
        return ":/icons/new-vault.svg";

    case IconId::OpenVault:
        return ":/icons/open-vault.svg";

    case IconId::SaveVault:
        return ":/icons/save-vault.svg";

    case IconId::SaveVaultAs:
        return ":/icons/save-vault-as.svg";

    case IconId::ExportXml:
        return ":/icons/export-xml.svg";

    case IconId::Exit:
        return ":/icons/exit.svg";

    case IconId::NewFolder:
        return ":/icons/new-folder.svg";

    case IconId::NewEntry:
        return ":/icons/new-entry.svg";

    case IconId::Rename:
        return ":/icons/rename.svg";

    case IconId::Delete:
        return ":/icons/delete.svg";

    case IconId::Cut:
        return ":/icons/cut.svg";

    case IconId::Copy:
        return ":/icons/copy.svg";

    case IconId::Paste:
        return ":/icons/paste.svg";

    case IconId::CollapseAll:
        return ":/icons/collapse-all.svg";

    case IconId::ExpandAll:
        return ":/icons/expand-all.svg";

    case IconId::Settings:
        return ":/icons/settings.svg";

    case IconId::ChangeMasterPassword:
        return ":/icons/change-master-password.svg";

    case IconId::Secrets:
        return ":/icons/secrets.svg";

    case IconId::AboutQt:
        return ":/icons/about-qt.svg";

    case IconId::CopySelected:
        return ":/icons/copy-selected.svg";

    case IconId::Reveal:
        return ":/icons/reveal.svg";

    case IconId::Stopwatch:
        return ":/icons/stopwatch.svg";

    case IconId::Hide:
        return ":/icons/hide.svg";

    case IconId::Edit:
        return ":/icons/edit.svg";

    case IconId::Save:
        return ":/icons/save.svg";

    case IconId::Maximize:
        return ":/icons/maximize.svg";

    case IconId::Minimize:
        return ":/icons/minimize.svg";

    case IconId::Quit:
        return ":/icons/quit.svg";
    }

    return {};
}