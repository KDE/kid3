/* Generated from kid3.h.both */
#ifndef KID3_H
#define KID3_H
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kmainwindow.h>
#else
#include <qmainwindow.h>
#endif

class KAction;
class KRecentFilesAction;
class KToggleAction;
class KURL;





class id3Form;
class Mp3File;
class FormatConfig;
class ImportConfig;
class MiscConfig;
class FreedbConfig;
class FrameList;
class StandardTags;



class Kid3App : public KMainWindow



{
Q_OBJECT

public:





        Kid3App(QWidget* parent=0, const char* name=0);



        ~Kid3App();





        void openDirectory(QString dir);






        void fileSelected(void);



        void updateCurrentSelection(void);





        void copyTags(const StandardTags *st);





        void pasteTags(StandardTags *st);





        void getTagsFromFilenameV1(void);





        void getTagsFromFilenameV2(void);







        void getFilenameFromTags(int tag_version);



        void copyV1ToV2(void);



        void copyV2ToV1(void);



        void removeTagsV1(void);



        void removeTagsV2(void);





        void openDrop(QString txt);



        void editFrame(void);



        void deleteFrame(void);



        void addFrame(void);


        FormatConfig *fnFormatCfg;

        FormatConfig *id3FormatCfg;

        ImportConfig *genCfg;

        MiscConfig *miscCfg;

        FreedbConfig *freedbCfg;

protected:



        void initActions();



        void initStatusBar();



        void initView();




        void cleanup();







        virtual bool queryClose();







        virtual void saveProperties(KConfig *_cfg);





        virtual void readProperties(KConfig *_cfg);
        void saveOptions();



        void readOptions();





        void setModified(bool val) { modified = val; }





        bool isModified(void) { return modified; }

public slots:



        void slotFileOpen();






        void slotFileOpenRecent(const KURL& url);



        void slotViewToolBar();



        void slotViewStatusBar();
        void slotFileRevert();



        void slotFileSave();



        void slotFileQuit();





        void slotStatusMsg(const QString &text);



        void slotCreatePlaylist(void);



        void slotImport(void);



        void slotSettingsConfigure(void);



        void slotApplyFormat(void);

private:





        bool saveDirectory(void);





        bool saveModified();





        void updateTags(Mp3File *mp3file);



        void updateModificationState(void);



        void updateAfterFrameModification(void);


        id3Form *view;


        bool modified;

        QString doc_dir;

        FrameList *framelist;

        StandardTags *copytags;



        KConfig* config;

        KAction* fileOpen;
        KRecentFilesAction* fileOpenRecent;
        KAction* fileRevert;
        KAction* fileSave;
        KAction* fileQuit;
        KToggleAction* viewToolBar;
        KToggleAction* viewStatusBar;
        KAction* settingsConfigure;
};
#endif
