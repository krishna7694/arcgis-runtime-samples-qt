// [WriteFile Name=Animate3DSymbols, Category=Scenes]
// [Legal]
// Copyright 2016 Esri.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// [Legal]

#include "Animate3DSymbols.h"
#include "MissionData.h"

#include "ArcGISTiledElevationSource.h"
#include "Camera.h"
#include "DistanceCompositeSceneSymbol.h"
#include "GraphicsOverlay.h"
#include "Map.h"
#include "MapQuickView.h"
#include "ModelSceneSymbol.h"
#include "PointCollection.h"
#include "Polyline.h"
#include "PolylineBuilder.h"
#include "Scene.h"
#include "SceneQuickView.h"
#include "SceneViewTypes.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleMarkerSceneSymbol.h"
#include "SimpleRenderer.h"
#include "SpatialReference.h"

#include <QFileInfo>
#include <QStringListModel>
#include <QQmlProperty>

using namespace Esri::ArcGISRuntime;

const QString Animate3DSymbols::HEADING = "HEADING";
const QString Animate3DSymbols::ROLL = "ROLL";
const QString Animate3DSymbols::PITCH = "PITCH";

Animate3DSymbols::Animate3DSymbols(QQuickItem* parent /* = nullptr */):
  QQuickItem(parent),
  m_sceneView(nullptr),
  m_mapView(nullptr),
  m_model3d(nullptr),
  m_graphic3d(nullptr),
  m_graphic2d(nullptr),
  m_routeGraphic(nullptr),
  m_missionModel( new QStringListModel({"Grand Canyon", "Hawaii", "Pyrenees", "Snowdon"}, this)),
  m_missionData( new MissionData()),
  m_frame(0),
  m_missionReady(false),
  m_zoomDist(0.),
  m_following(true)
{
}

Animate3DSymbols::~Animate3DSymbols()
{
}



void Animate3DSymbols::componentComplete()
{
  QQuickItem::componentComplete();

  // get the data path
  m_dataPath = QQmlProperty::read(this, "dataPath").toString();

  // find QML SceneView component
  m_sceneView = findChild<SceneQuickView*>("sceneView");

  // create a new scene instance
  Scene* scene = new Scene(Basemap::imagery(this), this);

  // set scene on the scene view
  m_sceneView->setScene(scene);

  // create a new elevation source
  ArcGISTiledElevationSource* elevationSource = new ArcGISTiledElevationSource(
    QUrl("http://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer"), this);

  // add the elevation source to the scene to display elevation
  scene->baseSurface()->elevationSources()->append(elevationSource);

  // create a new graphics overlay and add it to the sceneview
  GraphicsOverlay* sceneOverlay = new GraphicsOverlay(this);
  sceneOverlay->sceneProperties().setSurfacePlacement(SurfacePlacement::Absolute);
  m_sceneView->graphicsOverlays()->append(sceneOverlay);

  SimpleRenderer* renderer3D = new SimpleRenderer(this);
  RendererSceneProperties renderProperties = renderer3D->sceneProperties();
  renderProperties.setHeadingExpression(HEADING);
  renderProperties.setPitchExpression(PITCH);
  renderProperties.setRollExpression(ROLL);
  sceneOverlay->setRenderer(renderer3D);

  // find QML MapView component
  m_mapView = findChild<MapQuickView*>("mapView");

  // set up mini map
  Map* map = new Map(Basemap::imagery(this), this);
  m_mapView->setMap(map);

  // create a graphics overlay for the mini map
  GraphicsOverlay* mapOverlay = new GraphicsOverlay(this);
  m_mapView->graphicsOverlays()->append(mapOverlay);

  // create renderer to handle updating plane heading using the graphics card
  SimpleRenderer* renderer2D = new SimpleRenderer(this);
  renderer2D->setRotationExpression("[ANGLE]");
  mapOverlay->setRenderer(renderer2D);

  // set up route graphic
//  createRoute2d(mapOverlay);

  createModel2d(mapOverlay);

  changeMission(m_missionModel->data(m_missionModel->index(0,0)).toString());
  nextFrame();
}

void Animate3DSymbols::nextFrame()
{
  if(m_missionData == nullptr)
    return;

  if(m_frame < m_missionData->size())
  {
    const MissionData::DataPoint& dp = m_missionData->dataAt(m_frame);
    m_graphic3d->setGeometry(dp.m_pos);
    m_graphic3d->attributes()->replaceAttribute(HEADING, dp.m_heading);
    m_graphic3d->attributes()->replaceAttribute(PITCH, dp.m_pitch);
    m_graphic3d->attributes()->replaceAttribute(ROLL, dp.m_roll);
    m_sceneView->update();

//    if( m_frame%5 == 0)
//      m_graphic2d->setGeometry(newP);

    if(m_following)
    {

        Camera camera(dp.m_pos, m_zoomDist, dp.m_heading, m_angle, dp.m_roll );
        m_sceneView->setViewpointCameraAndWait(camera);

//      Point camP = m_sceneView->currentViewpointCamera().location();
//      double dist = GeometryEngine::distance(camP, dp.m_pos);
//      qDebug() << "dist = " << dist << " & zoom dist = " << m_zoomDist;
//      if( dist > m_zoomDist )
//      {
//        Camera camera(dp.m_pos, m_zoomDist, dp.m_heading, m_angle, dp.m_roll );
//        m_sceneView->setViewpointCameraAndWait(camera);
//      }
//      else
//         m_sceneView->update();

//    m_sceneView->setViewpointCamera(camera);
    }
    else
    {
//      m_graphic2d->attributes()->replaceAttribute("ANGLE", 360 + dp.m_heading - m_mapView->mapRotation());
       m_sceneView->update();
    }

  }

  m_frame++;
  if(m_frame >= m_missionData->size())
    m_frame = 0;
}

void Animate3DSymbols::changeMission(const QString &missionNameStr)
{
  m_frame = 0;
  QString formattedname = missionNameStr;
  formattedname.remove(" ");
  m_missionReady = m_missionData->parse( QUrl(m_dataPath + "/Missions/" + formattedname + ".csv").toLocalFile());

//  PolylineBuilder* routeBldr = new PolylineBuilder(SpatialReference::wgs84(), this);
//  for(size_t i = 0; i < m_missionData->size(); ++i )
//  {
//    const MissionData::DataPoint& dp = m_missionData->dataAt(i);
//    routeBldr->addPoint(dp.m_pos);
//  }

//  m_routeGraphic->setGeometry(routeBldr->toGeometry());

  if(m_missionReady)
  {
    const MissionData::DataPoint& dp = m_missionData->dataAt(m_frame);
    Camera camera(dp.m_pos, m_zoomDist, dp.m_heading, m_angle, dp.m_roll);
    m_sceneView->setViewpointCameraAndWait(camera);
    createModel3d();
  }

  emit missionReadyChanged();
}

void Animate3DSymbols::clearGraphic3D()
{
  if(m_graphic3d == nullptr)
    return;

  if(m_sceneView->graphicsOverlays()->size() > 0 && m_sceneView->graphicsOverlays()->at(0))
    m_sceneView->graphicsOverlays()->at(0)->graphics()->clear();

  delete m_graphic3d;
  m_graphic3d = nullptr;
}

void Animate3DSymbols::createModel3d()
{
  if( m_model3d == nullptr)
  {
    m_model3d = new ModelSceneSymbol(QUrl(m_dataPath + "/SkyCrane/SkyCrane.lwo"), 0.01f, this);
    m_model3d->setHeading(180.);
  }

  connect(m_model3d, &ModelSceneSymbol::loadStatusChanged, [this]()
    {
      if (m_model3d->loadStatus() == LoadStatus::Loaded)
        createGraphic3D();
    }
  );

  if( m_model3d->loadStatus() == LoadStatus::Loaded)
    createGraphic3D();
  else
    m_model3d->load();
}

void Animate3DSymbols::createModel2d(GraphicsOverlay *mapOverlay)
{
  // create a blue triangle symbol to represent the plane on the mini map
  SimpleMarkerSymbol* plane2DSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Triangle, Qt::blue, 10, this);

  // create a graphic with the symbol and attributes
  QVariantMap attributes;
  attributes.insert("ANGLE", 0.f);
  m_graphic2d = new Graphic(Point(0, 0, SpatialReference::wgs84()), attributes, plane2DSymbol);
  mapOverlay->graphics()->append(m_graphic2d);
}

void Animate3DSymbols::createRoute2d(GraphicsOverlay* mapOverlay)
{
  SimpleLineSymbol* routeSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, Qt::red, 2);
  m_routeGraphic = new Graphic(this);
  m_routeGraphic->setSymbol(routeSymbol);
  mapOverlay->graphics()->append(m_routeGraphic);
}

void Animate3DSymbols::createGraphic3D()
{
  if(m_model3d == nullptr || !m_missionReady || m_missionData->size() == 0)
    return;

  clearGraphic3D();

  // get the mission data for the frame
  const MissionData::DataPoint& dp = m_missionData->dataAt(m_frame);

  // create a graphic using the model symbol
  m_graphic3d = new Graphic(dp.m_pos, m_model3d, this);
  m_graphic3d->attributes()->insertAttribute(HEADING, dp.m_heading);
  m_graphic3d->attributes()->insertAttribute(PITCH, dp.m_pitch);
  m_graphic3d->attributes()->insertAttribute(ROLL, dp.m_roll);

  // add the graphic to the graphics overlay
  m_sceneView->graphicsOverlays()->at(0)->graphics()->append(m_graphic3d);
}

void Animate3DSymbols::setFollowing(bool following)
{
  m_following = following;
}

bool Animate3DSymbols::missionReady() const
{
  return m_missionReady;
}
