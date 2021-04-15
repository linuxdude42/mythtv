#ifndef MYTHTVMENU_H
#define MYTHTVMENU_H

// Qt
#include <QMetaType>
#include <QDomDocument>

// MythTV
#include "mythtvexp.h"

enum MenuCategory
{
    kMenuCategoryItem,
    kMenuCategoryItemlist,
    kMenuCategoryMenu
};

enum MenuShowContext
{
    kMenuShowActive,
    kMenuShowInactive,
    kMenuShowAlways
};

enum MenuCurrentContext
{
    kMenuCurrentDefault,
    kMenuCurrentActive,
    kMenuCurrentAlways
};

// These correlate to the four MythTVMenu objects that are allocated
// and stashed inside of a TV object. They are used to find the
// original menu again, instead of serializing/deserializing the
// entire MythTVMenu object to/from a QVariant.
enum MenuTypeId
{
    kMenuIdUnknown,
    kMenuIdPlayback,
    kMenuIdPlaybackCompact,
    kMenuIdCutlist,
    kMenuIdCutlistCompact,
};

class OSD;
class MythOSDDialogData;
class MythTVMenu;

class MythTVMenuItemContext
{
  public:
    bool AddButton(MythOSDDialogData* Menu, bool Active,const QString& Action,
                   const QString& DefaultTextActive, const QString& DefaultTextInactive,
                   bool IsMenu, const QString& TextArg) const;

    MythTVMenuItemContext(const MythTVMenu& Menu, const QDomNode& Node,
                          QString Name, MenuCurrentContext Current, bool Display);
    MythTVMenuItemContext(const MythTVMenu& Menu, const QDomNode& Node,
                          MenuShowContext Context, MenuCurrentContext Current,
                          QString Action, QString ActionText, bool Display);
    MythTVMenuItemContext(const MythTVMenu& Menu, const QDomNode& Node,
                          MenuShowContext Context, MenuCurrentContext Current,
                          QString Action, bool Display);

    const MythTVMenu&  m_menu;
    const QDomNode&    m_node;
    MenuCategory       m_category       { kMenuCategoryItem   };
    const QString      m_menuName;
    MenuShowContext    m_showContext    { kMenuShowActive     };
    MenuCurrentContext m_currentContext { kMenuCurrentDefault };
    const QString      m_action;
    const QString      m_actionText;
    bool               m_doDisplay      { false };
};

class MythTVMenuItemDisplayer
{
  public:
    virtual ~MythTVMenuItemDisplayer() = default;
    virtual bool MenuItemDisplay(const MythTVMenuItemContext& Context, MythOSDDialogData* Menu) = 0;
};

class MTV_PUBLIC MythTVMenu
{
  public:
    MythTVMenu() = default;
   ~MythTVMenu();
    static bool MatchesGroup(const QString& Name, const QString& Prefix,
                             MenuCategory Category, QString& OutPrefix);
    bool        LoadFromFile(MenuTypeId id, const QString& Filename, const QString& Menuname,
                             const char * TranslationContext, const QString& KeyBindingContext,
                             int IncludeLevel = 0);
    bool        LoadFromString(MenuTypeId id, const QString& Text, const QString& Menuname,
                               const char * TranslationContext, const QString& KeyBindingContext,
                               int IncludeLevel = 0);
    bool        IsLoaded() const;
    QDomElement GetRoot() const;
    QString     Translate(const QString& Text) const;
    bool        Show(const QDomNode& Node, const QDomNode& Selected,
                     MythTVMenuItemDisplayer& Displayer, MythOSDDialogData* Menu,
                     bool Display = true) const;
    QString     GetName() const;
    const char* GetTranslationContext() const;
    const QString& GetKeyBindingContext() const;
    friend class TV;

  private:
    void ProcessIncludes(QDomElement& Root, int IncludeLevel);

    MenuTypeId    m_id                 { kMenuIdUnknown };
    QDomDocument* m_document           { nullptr };
    const char*   m_translationContext { "" };
    QString       m_menuName;
    QString       m_keyBindingContext;
};

static const MythTVMenu dummy_menubase;

class MythTVMenuNodeTuple
{
  public:
    MythTVMenuNodeTuple(MenuTypeId Id, const QDomNode& Node);
    MythTVMenuNodeTuple();

    MenuTypeId m_id { kMenuIdUnknown };
    const QDomNode   m_node;
};

Q_DECLARE_METATYPE(MythTVMenuNodeTuple)

#endif
