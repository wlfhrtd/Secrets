#include "iconprovider.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QGuiApplication>

IconProvider::IconProvider()
{
    // m_uiColor = Qt::black;
    // m_systemColor = Qt::black;

    // rebuildCache();
}

void IconProvider::setUiColor(const QColor& color)
{
    if (m_uiColor == color)
    {
        return;
    }

    m_uiColor = color;

    rebuildCache();
}

void IconProvider::setSystemColor(const QColor& color)
{
    if (m_systemColor == color)
    {
        return;
    }

    m_systemColor = color;
}

QPixmap IconProvider::renderSvgColored(
    const QString& path,
    const QSize& size,
    const QColor& color) const
{
    const qreal dpr = qApp->devicePixelRatio();

    QSvgRenderer renderer(path);

    QPixmap pix(size * dpr);

    pix.setDevicePixelRatio(dpr);

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

    const QList<QSize> sizes =
        {
            QSize(16,16),
            QSize(24,24),
            QSize(32,32),
            QSize(48,48),
            QSize(64,64)
        };

    for (const QSize& size : sizes)
    {
        icon.addPixmap(renderSvgColored(
            path,
            size,
            color));
    }

    return icon;
}

QIcon IconProvider::ui(IconId id) const
{
    return recolorIcon(iconPath(id), m_uiColor);
}

QIcon IconProvider::system(IconId id) const
{
    return recolorIcon(iconPath(id), m_systemColor);
}

QIcon IconProvider::windowIcon() const
{
    return system(IconId::Secrets);
}

void IconProvider::rebuildCache()
{
    m_noteIcons.copySelected = ui(IconId::CopySelected);
    m_noteIcons.reveal       = ui(IconId::Reveal);
    m_noteIcons.stopwatch    = ui(IconId::Stopwatch);
    m_noteIcons.hide         = ui(IconId::Hide);
    m_noteIcons.edit         = ui(IconId::Edit);
    m_noteIcons.save         = ui(IconId::Save);
}

const NoteToolBarIcons& IconProvider::noteToolBarIcons() const
{
    return m_noteIcons;
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