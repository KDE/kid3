set(model_SRCS
  model/iabortable.cpp
  model/audioplayer.cpp
  model/commandformatreplacer.cpp
  model/commandstablemodel.cpp
  model/configtablemodel.cpp
  model/dirproxymodel.cpp
  model/dirrenamer.cpp
  model/downloadclient.cpp
  model/expressionparser.cpp
  model/externalprocess.cpp
  model/filefilter.cpp
  model/fileproxymodel.cpp
  model/fileproxymodeliterator.cpp
  model/bidirfileproxymodeliterator.cpp
  model/framelist.cpp
  model/frametablemodel.cpp
  model/iframeeditor.cpp
  model/kid3application.cpp
  model/modeliterator.cpp
  model/taggedfileiconprovider.cpp
  model/texttablemodel.cpp
  model/trackdatamodel.cpp
  model/checkablestringlistmodel.cpp
  model/tagsearcher.cpp
  model/timeeventmodel.cpp
  model/eventtimingcode.cpp
  model/tracknumbervalidator.cpp
  model/taggedfileselection.cpp
)

set(model_MOC_HDRS
  model/audioplayer.h
  model/commandstablemodel.h
  model/dirrenamer.h
  model/downloadclient.h
  model/externalprocess.h
  model/filefilter.h
  model/fileproxymodel.h
  model/fileproxymodeliterator.h
  model/bidirfileproxymodeliterator.h
  model/framelist.h
  model/frametablemodel.h
  model/kid3application.h
  model/trackdatamodel.h
  model/tagsearcher.h
  model/timeeventmodel.h
  model/taggedfileselection.h
)

if (HAVE_QTDBUS)
  set(model_SRCS ${model_SRCS} model/scriptinterface.cpp)
  set(model_MOC_HDRS ${model_MOC_HDRS} model/scriptinterface.h)
endif (HAVE_QTDBUS)
