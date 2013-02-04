set(model_SRCS
  model/audioplayer.cpp
  model/commandformatreplacer.cpp
  model/commandstablemodel.cpp
  model/configtablemodel.cpp
  model/dirlist.cpp
  model/dirproxymodel.cpp
  model/dirrenamer.cpp
  model/downloadclient.cpp
  model/expressionparser.cpp
  model/externalprocess.cpp
  model/filefilter.cpp
  model/fileproxymodel.cpp
  model/fileproxymodeliterator.cpp
  model/framelist.cpp
  model/frametablemodel.cpp
  model/iframeeditor.cpp
  model/kid3application.cpp
  model/modeliterator.cpp
  model/taggedfileiconprovider.cpp
  model/texttablemodel.cpp
  model/trackdatamodel.cpp
  model/checkablestringlistmodel.cpp
)

set(model_MOC_HDRS
  model/audioplayer.h
  model/commandstablemodel.h
  model/downloadclient.h
  model/externalprocess.h
  model/filefilter.h
  model/fileproxymodel.h
  model/fileproxymodeliterator.h
  model/frametablemodel.h
  model/kid3application.h
  model/trackdatamodel.h
)

if (HAVE_QTDBUS)
  set(model_SRCS ${model_SRCS} model/scriptinterface.cpp)
  set(model_MOC_HDRS ${model_MOC_HDRS} model/scriptinterface.h)
endif (HAVE_QTDBUS)
