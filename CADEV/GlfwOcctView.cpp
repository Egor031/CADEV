// MIT License
// 
// Copyright(c) 2023 Shing Liu
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//Нужно, чтобы отображались русские буквы
#pragma execution_character_set("utf-8")

#include "GlfwOcctView.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <AIS_Shape.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <TopoDS.hxx>
#include <gp_Elips.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <StlAPI_Writer.hxx>
#include <StlAPI_Reader.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <Geom_CartesianPoint.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <AIS_Line.hxx>
#include <iostream>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include <GLFW/glfw3.h>

namespace
{
    //! Convert GLFW mouse button into Aspect_VKeyMouse.
    static Aspect_VKeyMouse mouseButtonFromGlfw(int theButton)
    {
        switch (theButton)
        {
        case GLFW_MOUSE_BUTTON_LEFT:   return Aspect_VKeyMouse_LeftButton;
        case GLFW_MOUSE_BUTTON_RIGHT:  return Aspect_VKeyMouse_RightButton;
        case GLFW_MOUSE_BUTTON_MIDDLE: return Aspect_VKeyMouse_MiddleButton;
        }
        return Aspect_VKeyMouse_NONE;
    }

    //! Convert GLFW key modifiers into Aspect_VKeyFlags.
    static Aspect_VKeyFlags keyFlagsFromGlfw(int theFlags)
    {
        Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
        if ((theFlags & GLFW_MOD_SHIFT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_SHIFT;
        }
        if ((theFlags & GLFW_MOD_CONTROL) != 0)
        {
            aFlags |= Aspect_VKeyFlags_CTRL;
        }
        if ((theFlags & GLFW_MOD_ALT) != 0)
        {
            aFlags |= Aspect_VKeyFlags_ALT;
        }
        if ((theFlags & GLFW_MOD_SUPER) != 0)
        {
            aFlags |= Aspect_VKeyFlags_META;
        }
        return aFlags;
    }
}

// ================================================================
// Function : GlfwOcctView
// Purpose  :
// ================================================================
GlfwOcctView::GlfwOcctView()
{
}

// ================================================================
// Function : ~GlfwOcctView
// Purpose  :
// ================================================================
GlfwOcctView::~GlfwOcctView()
{
}

// ================================================================
// Function : toView
// Purpose  :
// ================================================================
GlfwOcctView* GlfwOcctView::toView(GLFWwindow* theWin)
{
    return static_cast<GlfwOcctView*>(glfwGetWindowUserPointer(theWin));
}

// ================================================================
// Function : errorCallback
// Purpose  :
// ================================================================
void GlfwOcctView::errorCallback(int theError, const char* theDescription)
{
    Message::DefaultMessenger()->Send(TCollection_AsciiString("Error") + theError + ": " + theDescription, Message_Fail);
}

// ================================================================
// Function : run
// Purpose  :
// ================================================================

void GlfwOcctView::run()
{
    initWindow(1230-50, 1030-50, "CADEV");
    initViewer();
    if (myView.IsNull())
    {
        return;
    }
    myView->MustBeResized();
    myOcctWindow->Map();
    initGui();
    mainloop();
    cleanup();
}

std::vector<TopoDS_Shape> history_vectorTopoDs;
std::vector<Handle(AIS_Shape)> history_vectorAIS;
static bool boxFlag = false;
static bool cylFlag = false;
static bool sphFlag = false;
static bool conFlag = false;
static bool cutFlag = false;
static bool fuseFlag = false;
static bool circFlag = false;
static bool extrFlag = false;
static bool revFlag = false;
static bool saveFlag = false;
static bool loadFlag = false;
static bool errorpathFlag = false;
static bool erroridFlag = false;
static double sdvig = 0;
char a[1000] = "История построения:\n";
TopoDS_Shape TPDS_Shape_Cut1;
TopoDS_Shape TPDS_Shape_Cut2;
TopoDS_Shape TPDS_Shape_Fuse1;
TopoDS_Shape TPDS_Shape_Fuse2;
int counter = 0;

std::vector<TopoDS_Edge> SketchAdd;
std::vector< Handle(AIS_Shape)> ShapeBuild;
static bool create = false;
static bool SketchMode = false;
static bool PointFlag = false;
static bool LineFlag = false;
static bool circleFlag = false;
static bool ArcFlag = false;
static bool RectangleFlag = false;
static bool ellipseFlag = false;
static bool polygonFlag = false;

/*
void GlfwOcctView::RenderInputDigitError()
{
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    ImVec2 windowSize(screenSize.x*0.05f, screenSize.y * 0.2f);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("Errors", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);

    ImGui::Begin("Error");
    ImGui::Text("Wrong input");
    if (ImGui::Button("OK"))
    {
        ; erroridFlag = false;
    }
    ImGui::End();
}
*/


bool demo_exists(static std::filesystem::path& p, std::filesystem::file_status s = std::filesystem::file_status{})
{
    std::cout << p;
    if (std::filesystem::status_known(s) ? std::filesystem::exists(s) : std::filesystem::exists(p))
        return(1);
    else
    {
        errorpathFlag = true;
        return(0);
    }
}

bool CheckChar(static char input[10])
{
    for (int i = 0; i < 10; i++)
        if (std::isdigit(input[i]) || input[i] == '.' || input[i] == '-')
        {
            return(true); std::cout << "1";
        }
        else
        {
            erroridFlag = true; return(false);
        }
}


void GlfwOcctView::MakeBox(static char xpoint[10],
    static char xlen[10],
    static char ypoint[10],
    static char ylen[10],
    static char zpoint[10],
    static char zlen[10])
{
    double xpointd, xlend, ypointd, ylend, zpointd, zlend;

    if (CheckChar(xpoint))
        if (CheckChar(ypoint))
            if (CheckChar(zpoint))
                if (CheckChar(xlen))
                    if (CheckChar(ylen))
                        if (CheckChar(zlen))
                        {
                            xpointd = std::stod(xpoint);
                            ypointd = std::stod(ypoint);
                            zpointd = std::stod(zpoint);
                            xlend = std::stod(xlen);
                            ylend = std::stod(ylen);
                            zlend = std::stod(zlen);
                            if (xlend > 0) {
                                if (ylend > 0) {
                                    if (zlend > 0)
                                    {
                                        myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.08, V3d_WIREFRAME);
                                        gp_Ax2 anAxis;
                                        anAxis.SetLocation(gp_Pnt(xpointd, ypointd, zpointd));
                                        TopoDS_Shape aBoxDS = BRepPrimAPI_MakeBox(anAxis, xlend, ylend, zlend);
                                        Handle(AIS_Shape) aBox = new AIS_Shape(aBoxDS);
                                        aBox->SetMaterial(Graphic3d_NOM_CHROME);
                                        myContext->Display(aBox, AIS_Shaded, 0, false);
                                        counter++;
                                        history_vectorTopoDs.push_back(aBoxDS);
                                        history_vectorAIS.push_back(aBox);
                                        strcat_s(a, "Параллелепипед\n");
                                    }
                                    else erroridFlag = true;
                                }
                                else erroridFlag = true;
                            }
                            else erroridFlag = true;
                        }
}

void GlfwOcctView::MakeCut(TopoDS_Shape CutShape1, TopoDS_Shape CutShape2)
{
    if (CutShape1.IsNull() || CutShape2.IsNull()) erroridFlag = true;
    else
    {
        for (static int i = 0; i < history_vectorAIS.size(); i++)
        {
            if (CutShape1 == history_vectorTopoDs[i]) myContext->Erase(history_vectorAIS[i], true);
            if (CutShape2 == history_vectorTopoDs[i]) myContext->Erase(history_vectorAIS[i], true);
        }
        TopoDS_Shape TPDS_Shape_CutRes = BRepAlgoAPI_Cut(CutShape1, CutShape2);
        Handle(AIS_Shape) Result = new AIS_Shape(TPDS_Shape_CutRes);
        Result->SetMaterial(Graphic3d_NOM_CHROME);
        myContext->Display(Result, AIS_Shaded, 0, false);
        counter++;
        history_vectorTopoDs.push_back(TPDS_Shape_CutRes);
        history_vectorAIS.push_back(Result);
        strcat_s(a, "Вычитание\n");
    }
}

void GlfwOcctView::MakeFuse(TopoDS_Shape FuseShape1, TopoDS_Shape FuseShape2)
{
    if (FuseShape1.IsNull() || FuseShape2.IsNull()) erroridFlag = true;
    else
    {
        for (static int i = 0; i < history_vectorAIS.size(); i++)
        {
            if (FuseShape1 == history_vectorTopoDs[i]) myContext->Erase(history_vectorAIS[i], true);
            if (FuseShape2 == history_vectorTopoDs[i]) myContext->Erase(history_vectorAIS[i], true);
        }

        TopoDS_Shape TPDS_Shape_FuseRes = BRepAlgoAPI_Fuse(FuseShape1, FuseShape2);
        Handle(AIS_Shape) Result = new AIS_Shape(TPDS_Shape_FuseRes);
        Result->SetMaterial(Graphic3d_NOM_CHROME);
        myContext->Display(Result, AIS_Shaded, 0, false);
        counter++;
        history_vectorTopoDs.push_back(TPDS_Shape_FuseRes);
        history_vectorAIS.push_back(Result);
        strcat_s(a, "Объединение\n");
    }
}

void GlfwOcctView::MakeCyl(static char xpoint[10],
    static char ypoint[10],
    static char zpoint[10],
    static char r[10],
    static char h[10],
    static char xdir[10],
    static char ydir[10],
    static char zdir[10])
{
    double xpointd, ypointd, zpointd, rd, hd, xdird, ydird, zdird;

    if (CheckChar(xpoint)) if (CheckChar(ypoint)) if (CheckChar(zpoint))
        if (CheckChar(r))
            if (CheckChar(h))
                if (CheckChar(xdir)) if (CheckChar(ydir)) if (CheckChar(zdir))
                {
                    xpointd = std::stod(xpoint);
                    ypointd = std::stod(ypoint);
                    zpointd = std::stod(zpoint);
                    rd = std::stod(r);
                    hd = std::stod(h);
                    xdird = std::stod(xdir);
                    ydird = std::stod(ydir);
                    zdird = std::stod(zdir);

                    if (xdird == 0 && ydird == 0 && zdird == 0) erroridFlag = true;
                    else
                        if (rd > 0)
                        {
                            if (hd > 0)
                            {
                                myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.08, V3d_WIREFRAME);
                                gp_Ax2 anAxis;
                                anAxis.SetLocation(gp_Pnt(xpointd, ypointd, zpointd));
                                gp_Dir anDir(xdird, ydird, zdird);
                                anAxis.SetDirection(anDir);
                                TopoDS_Shape aCylDS = BRepPrimAPI_MakeCylinder(anAxis, rd, hd);
                                Handle(AIS_Shape) aCyl = new AIS_Shape(aCylDS);
                                aCyl->SetMaterial(Graphic3d_NOM_CHROME);;
                                myContext->Display(aCyl, AIS_Shaded, 0, false);
                                counter++;
                                history_vectorTopoDs.push_back(aCylDS);
                                history_vectorAIS.push_back(aCyl);
                                strcat_s(a, "Цилиндр\n");
                            }
                            else erroridFlag = true;
                        }
                        else erroridFlag = true;
                }
}

void GlfwOcctView::MakeCon(static char xpoint[10],
    static char ypoint[10],
    static char zpoint[10],
    static char r1[10],
    static char r2[10],
    static char h[10],
    static char xdir[10],
    static char ydir[10],
    static char zdir[10])
{
    double xpointd, ypointd, zpointd, r1d, r2d, hd, xdird, ydird, zdird;

    if (CheckChar(xpoint)) if (CheckChar(ypoint)) if (CheckChar(zpoint))
        if (CheckChar(r1)) if (CheckChar(r2))
            if (CheckChar(h))
                if (CheckChar(xdir)) if (CheckChar(ydir)) if (CheckChar(zdir))
                {
                    xpointd = std::stod(xpoint);
                    ypointd = std::stod(ypoint);
                    zpointd = std::stod(zpoint);
                    r1d = std::stod(r1);
                    r2d = std::stod(r2);
                    hd = std::stod(h);
                    xdird = std::stod(xdir);
                    ydird = std::stod(ydir);
                    zdird = std::stod(zdir);
                    if (xdird == 0 && ydird == 0 && zdird == 0) erroridFlag = true;
                    else
                        if (r1d > 0)
                        {
                            if (r2d > 0)
                            {
                                if (hd > 0)
                                {
                                    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.08, V3d_WIREFRAME);
                                    gp_Ax2 anAxis;
                                    anAxis.SetLocation(gp_Pnt(xpointd, ypointd, zpointd));
                                    gp_Dir anDir(xdird, ydird, zdird);
                                    anAxis.SetDirection(anDir);
                                    TopoDS_Shape aConDS = BRepPrimAPI_MakeCone(anAxis, r1d, r2d, hd);
                                    Handle(AIS_Shape) aCyl = new AIS_Shape(aConDS);
                                    aCyl->SetMaterial(Graphic3d_NOM_CHROME);;
                                    myContext->Display(aCyl, AIS_Shaded, 0, false);
                                    counter++;
                                    history_vectorTopoDs.push_back(aConDS);
                                    history_vectorAIS.push_back(aCyl);
                                    strcat_s(a, "Конус\n");
                                }
                                else erroridFlag = true;
                            }
                            else erroridFlag = true;
                        }
                        else erroridFlag = true;
                }
}

void GlfwOcctView::MakeSph(static char xpoint[10],
    static char ypoint[10],
    static char zpoint[10],
    static char r[10])
{
    double xpointd, ypointd, zpointd, rd;

    if (CheckChar(xpoint)) if (CheckChar(ypoint)) if (CheckChar(zpoint))
        if (CheckChar(r))
        {
            xpointd = std::stod(xpoint);
            ypointd = std::stod(ypoint);
            zpointd = std::stod(zpoint);
            rd = std::stod(r);
            if (rd > 0)
            {
                myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_RED, 0.08, V3d_WIREFRAME);
                gp_Pnt aPoint(xpointd, ypointd, zpointd);
                TopoDS_Shape aSphDS = BRepPrimAPI_MakeSphere(aPoint, rd);
                Handle(AIS_Shape) aSph = new AIS_Shape(aSphDS);
                aSph->SetMaterial(Graphic3d_NOM_CHROME);
                myContext->Display(aSph, AIS_Shaded, 0, false);
                counter++;
                history_vectorTopoDs.push_back(aSphDS);
                history_vectorAIS.push_back(aSph);
                strcat_s(a, "Шар\n");
            }
            else erroridFlag = true;
        }
}



void GlfwOcctView::MakeCirc()
{
    gp_Circ gpCircle;
    gpCircle.SetRadius(15.);
    gpCircle.SetLocation(gp_Pnt(0, 0, 0));
    gpCircle.SetAxis(gp_Ax1(gp_Pnt(0, sdvig, 0), gp_Dir(1, 0, 0)));
    TopoDS_Edge cicleEdge = BRepBuilderAPI_MakeEdge(gpCircle);
    Handle(AIS_Shape) aisCicleEdge = new AIS_Shape(cicleEdge);
    aisCicleEdge->SetColor(Quantity_NOC_GREEN1);
    myContext->Display(aisCicleEdge, true);
    sdvig += 50;
}


void GlfwOcctView::MakeRevolute(static char value[10], static int xax, static int yax, static int zax, static char xpnt[10], static char ypnt[10], static char zpnt[10])
{
    double param = 0, xaxd = 0, yaxd = 0, zaxd = 0, xpntd = 0, ypntd = 0, zpntd = 0;

    if (CheckChar(xpnt)) if (CheckChar(ypnt)) if (CheckChar(zpnt))
        if (CheckChar(value))
            if (xax == 0 && yax == 0 && zax == 0) erroridFlag = true;
            else
            {
                param = std::stod(value);
                xpntd = std::stod(xpnt);
                ypntd = std::stod(ypnt);
                zpntd = std::stod(zpnt);

                if (param > 0)
                {
                    myContext->InitSelected();
                    TopoDS_Shape f = myContext->SelectedShape();
                    if (f.IsNull()) erroridFlag = true;
                    else
                    {

                        TopExp_Explorer edgeExp1(f, TopAbs_EDGE);
                        BRepBuilderAPI_MakeWire makeWr;
                        while (edgeExp1.More())
                        {
                            TopoDS_Edge surface1_edge1 = TopoDS::Edge(edgeExp1.Current());
                            makeWr.Add(surface1_edge1);
                            edgeExp1.Next();
                        }
                        TopoDS_Wire Wc1 = makeWr;
                        edgeExp1.Clear();
                        TopoDS_Face F1 = BRepBuilderAPI_MakeFace(Wc1);

                        gp_Ax1 ax1(gp_Pnt(xpntd, ypntd, zpntd), gp_Dir(xax, yax, zax));

                        TopoDS_Shape shape = BRepPrimAPI_MakeRevol(F1, ax1, (param * M_PI / 180), true);
                        Handle(AIS_Shape) aAISShape = new AIS_Shape(shape);
                        aAISShape->SetMaterial(Graphic3d_NOM_CHROME);
                        aAISShape->SetDisplayMode(AIS_Shaded);
                        myContext->Display(aAISShape, true);
                        /*
                        TopoDS_Shape shape2 = shape;
                        Handle(AIS_Shape) aAISShape22 = new AIS_Shape(shape2);
                        aAISShape22->SetDisplayMode(AIS_WireFrame);
                        aAISShape22->SetColor(Quantity_NOC_BLACK);
                        myContext->Display(aAISShape22, true);
                        */
                        history_vectorTopoDs.push_back(shape);
                        history_vectorAIS.push_back(aAISShape);
                        strcat_s(a, "Вращение\n");
                    }
                }
                else erroridFlag = true;
            }
}

void GlfwOcctView::MakeExtrude(static char value[10], static int xax, static int yax, static int zax)
{
    if (CheckChar(value))
    {
        double param = 0;
        param = std::stod(value);
        myContext->InitSelected();
        TopoDS_Shape f = myContext->SelectedShape();
        if (xax == 0 && yax == 0 && zax == 0) erroridFlag = true;
        else
            if (param > 0)
            {
                if (f.IsNull()) erroridFlag = true;
                else
                {
                    TopExp_Explorer edgeExp1(f, TopAbs_EDGE);
                    BRepBuilderAPI_MakeWire makeWr;
                    while (edgeExp1.More())
                    {
                        TopoDS_Edge surface1_edge1 = TopoDS::Edge(edgeExp1.Current());
                        makeWr.Add(surface1_edge1);
                        edgeExp1.Next();
                    }
                    TopoDS_Wire Wc1 = makeWr;
                    edgeExp1.Clear();
                    TopoDS_Face F1 = BRepBuilderAPI_MakeFace(Wc1);

                    gp_Vec move(gp_Dir(xax, yax, zax));
                    move *= param;

                    TopoDS_Shape shape = BRepPrimAPI_MakePrism(F1, move);
                    Handle(AIS_Shape) aAISShape = new AIS_Shape(shape);
                    aAISShape->SetMaterial(Graphic3d_NOM_CHROME);
                    aAISShape->SetDisplayMode(AIS_Shaded);
                    myContext->Display(aAISShape, true);

                    history_vectorTopoDs.push_back(shape);
                    history_vectorAIS.push_back(aAISShape);
                    strcat_s(a, "Вытягивание\n");
                }
            }
        else erroridFlag = true;
    }
}

void GlfwOcctView::Tempfunc()
{

    BRepBuilderAPI_MakeWire MakeWr;

    gp_Pnt line1start(0, 0, 0);
    gp_Pnt line1end(0, 0, 20);
    TopoDS_Edge line1 = BRepBuilderAPI_MakeEdge(line1start, line1end);
    MakeWr.Add(line1);

    gp_Pnt line2start = line1end;
    gp_Pnt line2end(0, 20, 20);
    TopoDS_Edge line2 = BRepBuilderAPI_MakeEdge(line2start, line2end);
    MakeWr.Add(line2);

    gp_Pnt line3start = line2end;
    gp_Pnt line3end(0, 20, 0);
    TopoDS_Edge line3 = BRepBuilderAPI_MakeEdge(line3start, line3end);
    MakeWr.Add(line3);

    gp_Pnt line4start = line3end;
    gp_Pnt line4end = line1start;
    TopoDS_Edge line4 = BRepBuilderAPI_MakeEdge(line4start, line4end);
    MakeWr.Add(line4);

    TopoDS_Wire Wr = MakeWr;
    Handle(AIS_Shape) lineShape = new AIS_Shape(Wr);
    lineShape->SetColor(Quantity_NOC_GREEN1);
    myContext->Display(lineShape, true);
}

void GlfwOcctView::MakeSave(static char path[100])
{
    int temp = 0;
    static char localPath[100];
    memset(localPath, NULL, 100);
    strcat_s(localPath, 100, path);
    for (int i = 0; i < 100; i++)
    {
        if (localPath[i] == '\\') temp = i;
    }
    temp--;
    for (int i = temp; i < 100; i++)
    {
        localPath[i] = NULL;
    }
    static std::filesystem::path sandbox{ localPath };
    if (demo_exists(sandbox))
    {
        strcat_s(path, 100, ".stl");
        AIS_ListOfInteractive myListObj;
        myContext->DisplayedObjects(myListObj);
        Handle(AIS_Shape) S = Handle(AIS_Shape)::DownCast(myListObj.First());
        TopoDS_Shape Sh = S->Shape();
        StlAPI_Writer stlWriter;
        stlWriter.Write(Sh, path);
    }

}

void GlfwOcctView::MakeLoad(static char path[100])
{
    static char localPath[100];
    memset(localPath, NULL, 100);
    strcat_s(localPath, 100, path);
    strcat_s(localPath, 100, ".stl");
    TopoDS_Shape shape;
    StlAPI_Reader stlReader;
    static std::filesystem::path sandbox{ localPath };
    if (demo_exists(sandbox))
    {
        stlReader.Read(shape, localPath);
        Handle(AIS_Shape) aAISShape = new AIS_Shape(shape);
        aAISShape->SetDisplayMode(AIS_Shaded);
        aAISShape->SetMaterial(Graphic3d_NOM_CHROME);
        myContext->Display(aAISShape, true);

    }

}

void GlfwOcctView::BuildPoint(gp_Pnt Point)
{

    Handle(AIS_Shape) TempBuildPoint;
    TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(Point);
    TempBuildPoint = new AIS_Shape(vertex);
    TempBuildPoint->SetColor(Quantity_NOC_YELLOW1);
    myContext->Display(TempBuildPoint, true);

}

void GlfwOcctView::BuildLine(gp_Pnt Point1, gp_Pnt Point2)
{
    TopoDS_Edge line1 = BRepBuilderAPI_MakeEdge(Point1, Point2);
    SketchAdd.push_back(line1);
    ShapeBuild.push_back(new AIS_Shape(SketchAdd.back()));
    myContext->Display(ShapeBuild.back(), true);
}

void GlfwOcctView::BuildCircle(gp_Pnt Point, gp_Dir Dir, double Radius)
{
    gp_Circ gpCircle;
    gpCircle.SetRadius(Radius);
    gpCircle.SetLocation(Point);
    gpCircle.SetAxis(gp_Ax1(Point, Dir));

    TopoDS_Edge gpCircleEdge = BRepBuilderAPI_MakeEdge(gpCircle);

    SketchAdd.push_back(gpCircleEdge);
    ShapeBuild.push_back(new AIS_Shape(SketchAdd.back()));

    myContext->Display(ShapeBuild.back(), true);
}

void GlfwOcctView::BuildElips(gp_Pnt Point, gp_Dir Dir, double MaxRadius, double MinRadius)
{
    gp_Elips gpElipse;
    gpElipse.SetLocation(Point);
    gpElipse.SetMajorRadius(MaxRadius);
    gpElipse.SetMinorRadius(MinRadius);
    gpElipse.SetAxis(gp_Ax1(Point, Dir));

    TopoDS_Edge gpCircleEdge = BRepBuilderAPI_MakeEdge(gpElipse);

    SketchAdd.push_back(gpCircleEdge);
    ShapeBuild.push_back(new AIS_Shape(SketchAdd.back()));

    myContext->Display(ShapeBuild.back(), true);
}

void GlfwOcctView::BuildArc(gp_Pnt Point1, gp_Pnt Point2, gp_Pnt Point3)
{
    Handle(Geom_TrimmedCurve) cicle = GC_MakeArcOfCircle(Point1, Point2, Point3);
    TopoDS_Edge cicleEdge = BRepBuilderAPI_MakeEdge(cicle);
    SketchAdd.push_back(cicleEdge);

    ShapeBuild.push_back(new AIS_Shape(SketchAdd.back()));
    myContext->Display(ShapeBuild.back(), true);

}

void GlfwOcctView::CloseSketch()
{
    if (ShapeBuild.size() != 0)
    {
        BRepBuilderAPI_MakeWire MakeWr;
        for (size_t i = 0; i < SketchAdd.size(); i++)
        {
            MakeWr.Add(SketchAdd[i]);
        }
        if (MakeWr.IsDone()) {
            for (size_t i = 0; i < ShapeBuild.size(); i++)
            {
                myContext->Erase(ShapeBuild[i], false);
            }


            TopoDS_Wire Wr = MakeWr;
            Handle(AIS_Shape) lineShape = new AIS_Shape(Wr);
            myContext->Display(lineShape, true);

        }

    }
    ShapeBuild.clear();
    SketchAdd.clear();
}

static bool CheckCreateSketch;
void GlfwOcctView::CheckSketch()
{
    if (ShapeBuild.size() != 0)
    {
        BRepBuilderAPI_MakeWire MakeWr;
        for (size_t i = 0; i < SketchAdd.size(); i++)
        {
            MakeWr.Add(SketchAdd[i]);
        }
        if (MakeWr.IsDone()) {
            CheckCreateSketch = true;
        }
        else
        {
            CheckCreateSketch = false;
        }

    }
    else
    {
        CheckCreateSketch = false;
    }
}

bool isNumeric(const char* str) {
    int len = strlen(str);
    if (len == 0) return false;
    bool hasDot = false;
    if (len == 1 && str[0] == '-')
    {
        return false;
    }
    for (int i = 0; i < len; ++i) {
        if (i == 0 && str[i] == '-') {
            // Если первый символ - минус, то это допустимо для отрицательных чисел
            continue;
        }
        if (isdigit(str[i])) {
            // Если текущий символ - цифра, продолжаем цикл
            continue;
        }
        else if (str[i] == '.' && !hasDot) {
            // Если текущий символ - точка и мы еще не встречали точку
            hasDot = true;
            continue;
        }
        else {
            // Если текущий символ не цифра и не точка, возвращаем false
            return false;
        }
    }
    // Если прошли цикл и не встретили недопустимых символов, возвращаем true
    return true;
}

// ================================================================
// Function : initWindow
// Purpose  :
// ================================================================
void GlfwOcctView::initWindow(int theWidth, int theHeight, const char* theTitle)
{
    glfwSetErrorCallback(GlfwOcctView::errorCallback);
    glfwInit();
    const bool toAskCoreProfile = true;
    if (toAskCoreProfile)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined (__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);
        //glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    }
    myOcctWindow = new GlfwOcctWindow(theWidth, theHeight, theTitle);
    glfwSetWindowUserPointer(myOcctWindow->getGlfwWindow(), this);

    // window callback
    glfwSetWindowSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onResizeCallback);
    glfwSetFramebufferSizeCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onFBResizeCallback);

    // mouse callback
    glfwSetScrollCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseScrollCallback);
    glfwSetMouseButtonCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseButtonCallback);
    glfwSetCursorPosCallback(myOcctWindow->getGlfwWindow(), GlfwOcctView::onMouseMoveCallback);
}

// ================================================================
// Function : initViewer
// Purpose  :
// ================================================================
void GlfwOcctView::initViewer()
{
    if (myOcctWindow.IsNull()
        || myOcctWindow->getGlfwWindow() == nullptr)
    {
        return;
    }

    Handle(OpenGl_GraphicDriver) aGraphicDriver
        = new OpenGl_GraphicDriver(myOcctWindow->GetDisplay(), Standard_False);
    aGraphicDriver->SetBuffersNoSwap(Standard_True);

    Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
    aViewer->SetDefaultLights();
    aViewer->SetLightOn();
    aViewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
    //Сетка
    //aViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
    myView = aViewer->CreateView();
    myView->SetImmediateUpdate(Standard_False);
    myView->SetWindow(myOcctWindow, myOcctWindow->NativeGlContext());

    //Счётчик фпс и других параметров
    //myView->ChangeRenderingParams().ToShowStats = Standard_True;

    //Цвет фонового простанства
    myView->SetBackgroundColor(Quantity_TOC_RGB, 0, 0, 0);

    myContext = new AIS_InteractiveContext(aViewer);
}

void GlfwOcctView::initGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& aIO = ImGui::GetIO();
    //aIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 16.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

    ImGui_ImplGlfw_InitForOpenGL(myOcctWindow->getGlfwWindow(), Standard_True);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Setup Dear ImGui style.
    //ImGui::StyleColorsClassic();
}

void GlfwOcctView::renderGui()
{
    ImGuiIO& aIO = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    //Окно инструменты
    {
        // Установка цвета фона
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.8f); // Новый цвет фона  

        // Установка размеров и расположения окна
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowSize(screenSize.x, screenSize.y * 0.2f);

        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Файл"))
            {
                if (ImGui::MenuItem("Открыть", "Ctrl+O")) { loadFlag = true; }
                if (ImGui::MenuItem("Сохранить", "Ctrl+S")) { saveFlag = true; }
                ImGui::EndMenu();
            }
            if (saveFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                static char Path[100] = { 0 };
                ImGui::Begin("Make Save");
                ImGui::Text("Enter value");
                ImGui::InputText("Path:", Path, 100);
                if (ImGui::Button("OK"))
                {
                    MakeSave(Path); saveFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) saveFlag = false;
                ImGui::End();
            }
            if (loadFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                static char Path[100] = { 0 };
                ImGui::Begin("Make Load");
                ImGui::Text("Enter value");
                ImGui::InputText("Path:", Path, 100);
                if (ImGui::Button("OK"))
                {
                    MakeLoad(Path); loadFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) loadFlag = false;
                ImGui::End();
            }



            ImGui::EndMenuBar();
        }
        if (SketchMode == false)
        {
            if (ImGui::Button("Create Sketch")) { SketchMode = true; }
            ImGui::SameLine();
            if (ImGui::Button("Box")) { boxFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Sphere")) { sphFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Cyl")) { cylFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Con")) { conFlag = true; }
            if (ImGui::Button("Cut")) { cutFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Fuse")) { fuseFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Make Revolute")) { revFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Make Extrude")) { extrFlag = true; }
            if (ImGui::Button("Delete")) { myContext->EraseSelected(true); }
            ImGui::SameLine();
            if (ImGui::Button("Make const Circul")) { circFlag = true; }
            ImGui::SameLine();
            if (ImGui::Button("Tempfunc")) { Tempfunc(); }

            if (boxFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                static char xpoint[10];
                static char xlen[10];
                static char ypoint[10];
                static char ylen[10];
                static char zpoint[10];
                static char zlen[10];
                ImGui::Begin("Make Box");
                ImGui::Text("Enter value");
                ImGui::InputText("xp", xpoint, IM_ARRAYSIZE(xpoint));
                ImGui::SameLine();
                ImGui::InputText("xl", xlen, IM_ARRAYSIZE(xlen));
                ImGui::InputText("yp", ypoint, IM_ARRAYSIZE(ypoint));
                ImGui::SameLine();
                ImGui::InputText("yl", ylen, IM_ARRAYSIZE(ylen));
                ImGui::InputText("zp", zpoint, IM_ARRAYSIZE(zpoint));
                ImGui::SameLine();
                ImGui::InputText("zl", zlen, IM_ARRAYSIZE(zlen));
                if (ImGui::Button("OK"))
                {
                    MakeBox(xpoint, xlen, ypoint, ylen, zpoint, zlen); boxFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) boxFlag = false;
                ImGui::End();
            }

            if (cylFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                static char xpoint[10];
                static char ypoint[10];
                static char zpoint[10];
                static char xdir[10];
                static char ydir[10];
                static char zdir[10];
                static char r[10];
                static char h[10];
                ImGui::Begin("Make Cylinder");
                ImGui::Text("Enter value");
                ImGui::InputText("xp", xpoint, IM_ARRAYSIZE(xpoint));
                ImGui::SameLine();
                ImGui::InputText("x vector", xdir, IM_ARRAYSIZE(xdir));
                ImGui::InputText("yp", ypoint, IM_ARRAYSIZE(ypoint));
                ImGui::SameLine();
                ImGui::InputText("y vector", ydir, IM_ARRAYSIZE(ydir));
                ImGui::InputText("zp", zpoint, IM_ARRAYSIZE(zpoint));
                ImGui::SameLine();
                ImGui::InputText("z vector", zdir, IM_ARRAYSIZE(zdir));
                ImGui::InputText("Radius", r, IM_ARRAYSIZE(r));
                ImGui::InputText("Height", h, IM_ARRAYSIZE(h));
                if (ImGui::Button("OK"))
                {
                    MakeCyl(xpoint, ypoint, zpoint, r, h, xdir, ydir, zdir); cylFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) cylFlag = false;
                ImGui::End();
            }
            if (conFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                static char xpoint[10];
                static char ypoint[10];
                static char zpoint[10];
                static char xdir[10];
                static char ydir[10];
                static char zdir[10];
                static char r1[10];
                static char r2[10];
                static char h[10];
                ImGui::Begin("Make Conus");
                ImGui::Text("Enter value");
                ImGui::InputText("xp", xpoint, IM_ARRAYSIZE(xpoint));
                ImGui::SameLine();
                ImGui::InputText("x vector", xdir, IM_ARRAYSIZE(xdir));
                ImGui::InputText("yp", ypoint, IM_ARRAYSIZE(ypoint));
                ImGui::SameLine();
                ImGui::InputText("y vector", ydir, IM_ARRAYSIZE(ydir));
                ImGui::InputText("zp", zpoint, IM_ARRAYSIZE(zpoint));
                ImGui::SameLine();
                ImGui::InputText("z vector", zdir, IM_ARRAYSIZE(zdir));
                ImGui::InputText("Radius1", r1, IM_ARRAYSIZE(r1));
                ImGui::SameLine();
                ImGui::InputText("Radius2", r2, IM_ARRAYSIZE(r2));
                ImGui::InputText("Height", h, IM_ARRAYSIZE(h));
                if (ImGui::Button("OK"))
                {
                    MakeCon(xpoint, ypoint, zpoint, r1, r2, h, xdir, ydir, zdir); conFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) conFlag = false;
                ImGui::End();
            }

            if (cutFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                ImGui::Begin("Make Cut Shape");
                ImGui::Text("Enter value");
                ImGui::Text("Select body 1 "); ImGui::SameLine(); if (ImGui::Button("Confirm1"))
                {
                    myContext->InitSelected();
                    TPDS_Shape_Cut1 = myContext->SelectedShape();
                }
                ImGui::Text("Select body 2 "); ImGui::SameLine(); if (ImGui::Button("Confirm2"))
                {
                    myContext->InitSelected();
                    TPDS_Shape_Cut2 = myContext->SelectedShape();
                }
                if (ImGui::Button("OK")) { MakeCut(TPDS_Shape_Cut1, TPDS_Shape_Cut2); cutFlag = false; }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) cutFlag = false;
                ImGui::End();
            }

            if (fuseFlag)
            {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                ImGui::Begin("Make Fuse Shape");
                ImGui::Text("Enter value");
                ImGui::Text("Select body 1 "); ImGui::SameLine(); if (ImGui::Button("Confirm1"))
                {
                    myContext->InitSelected();
                    TPDS_Shape_Fuse1 = myContext->SelectedShape();
                }
                ImGui::Text("Select body 2 "); ImGui::SameLine(); if (ImGui::Button("Confirm2"))
                {
                    myContext->InitSelected();
                    TPDS_Shape_Fuse2 = myContext->SelectedShape();
                }
                if (ImGui::Button("OK")) { MakeFuse(TPDS_Shape_Fuse1, TPDS_Shape_Fuse2); fuseFlag = false; }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) fuseFlag = false;
                ImGui::End();
            }

            if (sphFlag)
            {
                static char xpoint[10];
                static char ypoint[10];
                static char zpoint[10];
                static char r[10];
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                ImGui::Begin("Make Sphere");
                ImGui::Text("Enter value");
                ImGui::InputText("xp", xpoint, IM_ARRAYSIZE(xpoint));
                ImGui::InputText("yp", ypoint, IM_ARRAYSIZE(ypoint));
                ImGui::InputText("zp", zpoint, IM_ARRAYSIZE(zpoint));
                ImGui::InputText("Radius", r, IM_ARRAYSIZE(r));
                if (ImGui::Button("OK")) { MakeSph(xpoint, ypoint, zpoint, r); sphFlag = false; }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) sphFlag = false;
                ImGui::End();
            }

            if (circFlag)
            {
                MakeCirc();
                circFlag = false;
            }

            if (revFlag)
            {
                static bool xax = 0;
                static bool yax = 0;
                static bool zax = 0;
                static char value[10];
                static char xpnt[10];
                static char ypnt[10];
                static char zpnt[10];
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                ImGui::Begin("Make Revolute");
                ImGui::Text("Revolute at:");
                ImGui::Checkbox("X axel", &xax);
                ImGui::SameLine();
                ImGui::Checkbox("Y axel", &yax);
                ImGui::SameLine();
                ImGui::Checkbox("Z axel", &zax);
                ImGui::InputText("Value", value, IM_ARRAYSIZE(value));
                ImGui::InputText("Start point x", xpnt, IM_ARRAYSIZE(xpnt));
                //ImGui::SameLine();
                ImGui::InputText("Start point y", ypnt, IM_ARRAYSIZE(ypnt));
                //ImGui::SameLine();
                ImGui::InputText("Start point z", zpnt, IM_ARRAYSIZE(zpnt));
                if (ImGui::Button("OK"))
                {

                    MakeRevolute(value, xax, yax, zax, xpnt, ypnt, zpnt);
                    revFlag = false;
                    xax = 0;
                    yax = 0;
                    zax = 0;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) revFlag = false;
                ImGui::End();
            }

            if (extrFlag)
            {
                static bool xax = 0;
                static bool yax = 0;
                static bool zax = 0;
                static bool xin = 0;
                static bool yin = 0;
                static bool zin = 0;
                static int xa = 0;
                static int ya = 0;
                static int za = 0;
                static char value[10];
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;
                colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
                ImGui::Begin("Make Revolute");
                ImGui::Text("Revolute at:");
                ImGui::Checkbox("X axel  ", &xax);
                ImGui::SameLine();
                ImGui::Checkbox("Y axel  ", &yax);
                ImGui::SameLine();
                ImGui::Checkbox("Z axel  ", &zax);

                ImGui::Checkbox("X invert", &xin);
                ImGui::SameLine();
                ImGui::Checkbox("Y invert", &yin);
                ImGui::SameLine();
                ImGui::Checkbox("Z invert", &zin);
                ImGui::InputText("Value", value, IM_ARRAYSIZE(value));
                if (ImGui::Button("OK"))
                {
                    if (xax) xa = 1; else if (xin) xa = -1; else xa = 0;
                    if (yax) ya = 1; else if (yin) ya = -1; else ya = 0;
                    if (zax) za = 1; else if (zin) za = -1; else za = 0;
                    MakeExtrude(value, xa, ya, za);
                    extrFlag = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) extrFlag = false;
                ImGui::End();
            }
            if (erroridFlag)
            {
                ImGui::Begin("Error");
                ImGui::Text("Wrong input");
                if (ImGui::Button("OK"))
                {
                    ; erroridFlag = false;
                }
                ImGui::End();
            }
            if (errorpathFlag)
            {
                ImGui::Begin("Error");
                ImGui::Text("Path error");
                if (ImGui::Button("OK"))
                {
                    ; errorpathFlag = false;
                }
                ImGui::End();
            }
        }
        else
        {
            if (ImGui::Button("Точка")) { PointFlag = true; }
            if (PointFlag)
            {
                ;
                static char Xpoint[10];
                static char Ypoint[10];
                static char Zpoint[10];
                static double Xoldpoint;
                static double Yoldpoint;
                static double Zoldpoint;
                static double xConvertPoint;
                static double yConvertPoint;
                static double zConvertPoint;
                static gp_Pnt tempPoint;
                static Handle(AIS_Shape) TempBuildPoint;
                ImGui::Begin("Построить точку");
                ImGui::InputText("X", Xpoint, IM_ARRAYSIZE(Xpoint));
                ImGui::InputText("Y", Ypoint, IM_ARRAYSIZE(Ypoint));
                //ImGui::InputText("Z", Zpoint, IM_ARRAYSIZE(Zpoint));
                //if (isNumeric(Xpoint) && isNumeric(Ypoint) && isNumeric(Zpoint))
                if (isNumeric(Xpoint) && isNumeric(Ypoint))
                {
                    xConvertPoint = std::stod(Xpoint);
                    yConvertPoint = std::stod(Ypoint);
                    //zConvertPoint = std::stod(Zpoint);
                    //zConvertPoint = 0;
                }

                /*if (create == false)
                {
                    tempPoint = gp_Pnt(xConvertPoint, yConvertPoint, zConvertPoint);

                    TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(tempPoint);
                    TempBuildPoint = new AIS_Shape(vertex);
                    TempBuildPoint->SetColor(Quantity_NOC_YELLOW1);
                    myContext->Display(TempBuildPoint, true);
                    create = true;
                }
                if (Xoldpoint != xConvertPoint || Yoldpoint != yConvertPoint || Zoldpoint != zConvertPoint)
                {
                    myContext->Erase(TempBuildPoint, true);
                    create = false;
                    Xoldpoint = xConvertPoint;
                    Yoldpoint = yConvertPoint;
                    Zoldpoint = zConvertPoint;
                }*/

                if (create == false || Xoldpoint != xConvertPoint || Yoldpoint != yConvertPoint) {
                    if (create) {
                        myContext->Erase(TempBuildPoint, true);
                    }
                    tempPoint = gp_Pnt(xConvertPoint, yConvertPoint, 0); // Задаем координату Z нулевой
                    TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(tempPoint);
                    TempBuildPoint = new AIS_Shape(vertex);
                    TempBuildPoint->SetColor(Quantity_NOC_YELLOW1);
                    myContext->Display(TempBuildPoint, true);
                    create = true;
                    Xoldpoint = xConvertPoint;
                    Yoldpoint = yConvertPoint;
                }

                if (ImGui::Button("OK"))
                {
                    myContext->Erase(TempBuildPoint, true);
                    BuildPoint(tempPoint); PointFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(TempBuildPoint, true);
                    PointFlag = false;
                    create = false;
                }
                ImGui::End();
            }
            ImGui::SameLine();
            if (ImGui::Button("Отрезок")) { LineFlag = true; }
            if (LineFlag)
            {
                static char Xpoint1[10];
                static char Ypoint1[10];
                static char Zpoint1[10];

                static double Xoldpoint1;
                static double Yoldpoint1;
                static double Zoldpoint1;
                static double Xoldpoint2;
                static double Yoldpoint2;
                static double Zoldpoint2;

                static double xConvertPoint1;
                static double yConvertPoint1;
                static double zConvertPoint1;
                static double xConvertPoint2;
                static double yConvertPoint2;
                static double zConvertPoint2;

                static char Xpoint2[10];
                static char Ypoint2[10];
                static char Zpoint2[10];

                static gp_Pnt tempPoint1;
                static gp_Pnt tempPoint2;
                static Handle(AIS_Line) lineVision;
                ImGui::Begin("Построить отрезок");

                ImGui::Columns(2, nullptr, false);

                ImGui::InputText("X1", Xpoint1, IM_ARRAYSIZE(Xpoint1));
                ImGui::InputText("Y1", Ypoint1, IM_ARRAYSIZE(Ypoint1));
                //ImGui::InputText("Z1", Zpoint1, IM_ARRAYSIZE(Zpoint1));

                ImGui::NextColumn();

                ImGui::InputText("X2", Xpoint2, IM_ARRAYSIZE(Xpoint2));
                ImGui::InputText("Y2", Ypoint2, IM_ARRAYSIZE(Ypoint2));
                //ImGui::InputText("Z2", Zpoint2, IM_ARRAYSIZE(Zpoint2));

                //if (isNumeric(Xpoint1) && isNumeric(Ypoint1) && isNumeric(Zpoint1) && isNumeric(Xpoint2) && isNumeric(Ypoint2) && isNumeric(Zpoint2))
                if (isNumeric(Xpoint1) && isNumeric(Ypoint1) && isNumeric(Xpoint2) && isNumeric(Ypoint2))
                {
                    xConvertPoint1 = std::stod(Xpoint1);
                    yConvertPoint1 = std::stod(Ypoint1);
                    //zConvertPoint1 = std::stod(Zpoint1);
                    zConvertPoint1 = 0;

                    xConvertPoint2 = std::stod(Xpoint2);
                    yConvertPoint2 = std::stod(Ypoint2);
                    //zConvertPoint2 = std::stod(Zpoint2);
                    zConvertPoint2 = 0;

                    if (create == false)
                    {
                        if (xConvertPoint1 != xConvertPoint2 || yConvertPoint1 != yConvertPoint2 || zConvertPoint1 != zConvertPoint2)
                        {
                            /*tempPoint1 = gp_Pnt(xConvertPoint1, yConvertPoint1, );
                        tempPoint2 = gp_Pnt(xConvertPoint2, yConvertPoint2, zConvertPoint2);*/

                        /* TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(tempPoint);
                         TempBuildPoint = new AIS_Shape(vertex);
                         TempBuildPoint->SetColor(Quantity_NOC_YELLOW1);
                         myContext->Display(TempBuildPoint, true);*/

                            tempPoint1 = gp_Pnt(xConvertPoint1, yConvertPoint1, zConvertPoint1);
                            tempPoint2 = gp_Pnt(xConvertPoint2, yConvertPoint2, zConvertPoint2);

                            Handle(Geom_CartesianPoint) p1 = new Geom_CartesianPoint(tempPoint1);
                            Handle(Geom_CartesianPoint) p2 = new Geom_CartesianPoint(tempPoint2);

                            lineVision = new AIS_Line(p1, p2);
                            myContext->Display(lineVision, true);
                            create = true;

                        }

                    }
                }

                if (Xoldpoint1 != xConvertPoint1 || Yoldpoint1 != yConvertPoint1 || Zoldpoint1 != zConvertPoint1 || Xoldpoint2 != xConvertPoint2 || Yoldpoint2 != yConvertPoint2 || Zoldpoint2 != zConvertPoint2)
                {
                    myContext->Erase(lineVision, true);
                    create = false;
                    Xoldpoint1 = xConvertPoint1;
                    Yoldpoint1 = yConvertPoint1;
                    Zoldpoint1 = zConvertPoint1;
                    Xoldpoint2 = xConvertPoint2;
                    Yoldpoint2 = yConvertPoint2;
                    Zoldpoint2 = zConvertPoint2;
                }
                if (ImGui::Button("OK"))
                {
                    myContext->Erase(lineVision, true);
                    BuildPoint(tempPoint1);
                    BuildPoint(tempPoint2);
                    BuildLine(tempPoint1, tempPoint2); LineFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(lineVision, true);
                    LineFlag = false;
                    create = false;
                }

                ImGui::End();
            }
            ImGui::SameLine();
            if (ImGui::Button("Прямоугольник")) { RectangleFlag = true; }
            if (RectangleFlag)
            {
                static char Xpoint1[10];
                static char Ypoint1[10];
                static char Zpoint1[10];

                static double Xoldpoint1;
                static double Yoldpoint1;
                static double Zoldpoint1;
                static double Xoldpoint2;
                static double Yoldpoint2;
                static double Zoldpoint2;

                static double xConvertPoint1;
                static double yConvertPoint1;
                static double zConvertPoint1;
                static double xConvertPoint2;
                static double yConvertPoint2;
                static double zConvertPoint2;

                static char Xpoint2[10];
                static char Ypoint2[10];
                static char Zpoint2[10];

                static gp_Pnt tempPoint1;
                static gp_Pnt tempPoint2;
                static gp_Pnt tempPoint3;
                static gp_Pnt tempPoint4;
                static Handle(AIS_Line) lineVision1;
                static Handle(AIS_Line) lineVision2;
                static Handle(AIS_Line) lineVision3;
                static Handle(AIS_Line) lineVision4;
                ImGui::Begin("Построить прямоугольник");

                ImGui::Columns(2, nullptr, false);

                ImGui::InputText("X1", Xpoint1, IM_ARRAYSIZE(Xpoint1));
                ImGui::InputText("Y1", Ypoint1, IM_ARRAYSIZE(Ypoint1));
                //ImGui::InputText("Z1", Zpoint1, IM_ARRAYSIZE(Zpoint1));

                ImGui::NextColumn();

                ImGui::InputText("X2", Xpoint2, IM_ARRAYSIZE(Xpoint2));
                ImGui::InputText("Y2", Ypoint2, IM_ARRAYSIZE(Ypoint2));
                //ImGui::InputText("Z2", Zpoint2, IM_ARRAYSIZE(Zpoint2));

                if (isNumeric(Xpoint1) && isNumeric(Ypoint1) && isNumeric(Xpoint2) && isNumeric(Ypoint2))
                {
                    xConvertPoint1 = std::stod(Xpoint1);
                    yConvertPoint1 = std::stod(Ypoint1);
                    //zConvertPoint1 = std::stod(Zpoint1);
                    zConvertPoint1 = 0;

                    xConvertPoint2 = std::stod(Xpoint2);
                    yConvertPoint2 = std::stod(Ypoint2);
                    //zConvertPoint2 = std::stod(Zpoint2);
                    zConvertPoint2 = 0;

                    if (create == false)
                    {
                        if (xConvertPoint1 != xConvertPoint2 || yConvertPoint1 != yConvertPoint2 || zConvertPoint1 != zConvertPoint2)
                        {
                            /*tempPoint1 = gp_Pnt(xConvertPoint1, yConvertPoint1, );
                        tempPoint2 = gp_Pnt(xConvertPoint2, yConvertPoint2, zConvertPoint2);*/

                        /* TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(tempPoint);
                         TempBuildPoint = new AIS_Shape(vertex);
                         TempBuildPoint->SetColor(Quantity_NOC_YELLOW1);
                         myContext->Display(TempBuildPoint, true);*/

                            tempPoint1 = gp_Pnt(xConvertPoint1, yConvertPoint1, zConvertPoint1);//0 0
                            tempPoint2 = gp_Pnt(xConvertPoint2, yConvertPoint2, zConvertPoint2);//10 10
                            tempPoint3 = gp_Pnt(xConvertPoint1, yConvertPoint2, zConvertPoint2);//0 10
                            tempPoint4 = gp_Pnt(xConvertPoint2, yConvertPoint1, zConvertPoint2);//10 0

                            Handle(Geom_CartesianPoint) p1 = new Geom_CartesianPoint(tempPoint1);
                            Handle(Geom_CartesianPoint) p2 = new Geom_CartesianPoint(tempPoint2);
                            Handle(Geom_CartesianPoint) p3 = new Geom_CartesianPoint(tempPoint3);
                            Handle(Geom_CartesianPoint) p4 = new Geom_CartesianPoint(tempPoint4);

                            lineVision1 = new AIS_Line(p1, p4);
                            lineVision2 = new AIS_Line(p4, p2);
                            lineVision3 = new AIS_Line(p2, p3);
                            lineVision4 = new AIS_Line(p3, p1);
                            myContext->Display(lineVision1, true);
                            myContext->Display(lineVision2, true);
                            myContext->Display(lineVision3, true);
                            myContext->Display(lineVision4, true);
                            create = true;

                        }

                    }
                }

                if (Xoldpoint1 != xConvertPoint1 || Yoldpoint1 != yConvertPoint1 || Zoldpoint1 != zConvertPoint1 || Xoldpoint2 != xConvertPoint2 || Yoldpoint2 != yConvertPoint2 || Zoldpoint2 != zConvertPoint2)
                {
                    myContext->Erase(lineVision1, true);
                    myContext->Erase(lineVision2, true);
                    myContext->Erase(lineVision3, true);
                    myContext->Erase(lineVision4, true);
                    create = false;
                    Xoldpoint1 = xConvertPoint1;
                    Yoldpoint1 = yConvertPoint1;
                    Zoldpoint1 = zConvertPoint1;
                    Xoldpoint2 = xConvertPoint2;
                    Yoldpoint2 = yConvertPoint2;
                    Zoldpoint2 = zConvertPoint2;
                }
                if (ImGui::Button("OK"))
                {
                    myContext->Erase(lineVision1, true);
                    myContext->Erase(lineVision2, true);
                    myContext->Erase(lineVision3, true);
                    myContext->Erase(lineVision4, true);
                    BuildPoint(tempPoint1);
                    BuildPoint(tempPoint2);
                    BuildPoint(tempPoint3);
                    BuildPoint(tempPoint4);
                    BuildLine(tempPoint1, tempPoint4);
                    BuildLine(tempPoint4, tempPoint2);
                    BuildLine(tempPoint2, tempPoint3);
                    BuildLine(tempPoint3, tempPoint1);
                    RectangleFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(lineVision1, true);
                    myContext->Erase(lineVision2, true);
                    myContext->Erase(lineVision3, true);
                    myContext->Erase(lineVision4, true);
                    RectangleFlag = false;
                    create = false;
                }

                ImGui::End();
            }
            ImGui::SameLine();
            if (ImGui::Button("Многоугольник")) { polygonFlag = true; }
            if (polygonFlag)
            {
                static char XCenPoint[10];
                static char YCenPoint[10];
                static char ZCenPoint[10];
                static double xConvertPoint;
                static double yConvertPoint;
                static double zConvertPoint;
                static double Xoldpoint;
                static double Yoldpoint;
                static double Zoldpoint;

                static char Radius[10];
                static double RadiusConvert;
                static double RadiusOld;

                static char countPoint[10];
                static int ConvertCountPoint;
                static int oldCountPoint;

                static std::vector<gp_Pnt> VecPoint;
                static std::vector<Handle(AIS_Line)> HandLine;

                ImGui::Begin("Построить многоугольник");

                ImGui::Columns(2, nullptr, false);

                ImGui::InputText("X1", XCenPoint, IM_ARRAYSIZE(XCenPoint));
                ImGui::InputText("Y1", YCenPoint, IM_ARRAYSIZE(YCenPoint));
                //ImGui::InputText("Z1", Zpoint1, IM_ARRAYSIZE(Zpoint1));

                ImGui::NextColumn();

                ImGui::InputText("R", Radius, IM_ARRAYSIZE(Radius));
                ImGui::InputText("Граней", countPoint, IM_ARRAYSIZE(countPoint));

                //ImGui::InputText("Z2", Zpoint2, IM_ARRAYSIZE(Zpoint2));

                if (isNumeric(XCenPoint) && isNumeric(YCenPoint) && isNumeric(Radius) && isNumeric(countPoint))
                {
                    xConvertPoint = std::stod(XCenPoint);
                    yConvertPoint = std::stod(YCenPoint);
                    //zConvertPoint1 = std::stod(Zpoint1);
                    zConvertPoint = 0;

                    RadiusConvert = std::stod(Radius);
                    ConvertCountPoint = std::stod(countPoint);

                    if (create == false)
                    {
                        if (ConvertCountPoint > 2) {


                            VecPoint.clear();
                            double angleIncrement = 2 * M_PI / ConvertCountPoint;
                            for (int i = 0; i < ConvertCountPoint; ++i) {
                                double angle = i * angleIncrement;
                                double x = xConvertPoint + RadiusConvert * cos(angle);
                                double y = yConvertPoint + RadiusConvert * sin(angle);
                                double z = 0.0; // Координата Z приравнивается к нулю

                                gp_Pnt point(x, y, z);
                                VecPoint.push_back(point); // Добавить точку в вектор


                            }
                            std::vector<Handle(Geom_CartesianPoint)> CartPoint;
                            for (int i = 0; i < ConvertCountPoint; i++)
                            {
                                CartPoint.push_back(new Geom_CartesianPoint(VecPoint[i]));
                            }
                            HandLine.clear();
                            for (int i = 0; i < ConvertCountPoint - 1; i++)
                            {
                                HandLine.push_back(new AIS_Line(CartPoint[i], CartPoint[i + 1]));
                            }
                            HandLine.push_back(new AIS_Line(CartPoint[0], CartPoint[CartPoint.size() - 1]));
                            for (int i = 0; i < HandLine.size(); i++)
                            {
                                myContext->Display(HandLine[i], true);
                            }
                            create = true;
                        }
                    }
                }
                if (Xoldpoint != xConvertPoint || Yoldpoint != yConvertPoint || Zoldpoint != zConvertPoint || RadiusOld != RadiusConvert || oldCountPoint != ConvertCountPoint)
                {
                    for (int i = 0; i < HandLine.size(); i++)
                    {
                        myContext->Erase(HandLine[i], true);
                    }
                    create = false;
                    Xoldpoint = xConvertPoint;
                    Yoldpoint = yConvertPoint;
                    Zoldpoint = zConvertPoint;
                    oldCountPoint = ConvertCountPoint;
                    RadiusOld = RadiusConvert;
                }
                if (ImGui::Button("OK"))
                {
                    for (int i = 0; i < HandLine.size(); i++)
                    {
                        myContext->Erase(HandLine[i], true);
                    }
                    for (int i = 0; i < VecPoint.size(); i++)
                    {
                        BuildPoint(VecPoint[i]);
                    }
                    for (int i = 0; i < VecPoint.size() - 1; i++)
                    {
                        BuildLine(VecPoint[i], VecPoint[i + 1]);
                    }
                    BuildLine(VecPoint[0], VecPoint[VecPoint.size() - 1]);
                    polygonFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    for (int i = 0; i < HandLine.size(); i++)
                    {
                        myContext->Erase(HandLine[i], true);
                    }
                    polygonFlag = false;
                    create = false;
                }

                ImGui::End();
            }
            if (ImGui::Button("Окружность")) { circleFlag = true; }
            if (circleFlag)
            {
                static char Xpoint[10];
                static char Ypoint[10];
                static char Zpoint[10];
                static double xConvertPoint;
                static double yConvertPoint;
                static double zConvertPoint;
                static double Xoldpoint;
                static double Yoldpoint;
                static double Zoldpoint;

                static char XDirpoint[10];
                static char YDirpoint[10];
                static char ZDirpoint[10];
                static double xDirConvertPoint;
                static double yDirConvertPoint;
                static double zDirConvertPoint;
                static double XDiroldpoint;
                static double YDiroldpoint;
                static double ZDiroldpoint;

                static char Radius[10];
                static double RadiusConvert;
                static double RadiusOld;

                static gp_Pnt Point;
                static gp_Dir Dir;

                static Handle(AIS_Shape) aisCicleEdge;

                ImGui::Begin("Построить окружность");
                ImGui::Columns(2, nullptr, false);

                ImGui::InputText("X1", Xpoint, IM_ARRAYSIZE(Xpoint));
                ImGui::InputText("Y1", Ypoint, IM_ARRAYSIZE(Ypoint));
                //ImGui::InputText("Z1", Zpoint, IM_ARRAYSIZE(Zpoint));
                ImGui::InputText("Радиус", Radius, IM_ARRAYSIZE(Radius));

                ImGui::NextColumn();

                //ImGui::InputText("DX2", XDirpoint, IM_ARRAYSIZE(XDirpoint));
                //ImGui::InputText("DY2", YDirpoint, IM_ARRAYSIZE(YDirpoint));
                //ImGui::InputText("DZ2", ZDirpoint, IM_ARRAYSIZE(ZDirpoint));

                /*if (isNumeric(Xpoint) && isNumeric(Ypoint) && isNumeric(Zpoint) && isNumeric(XDirpoint) && isNumeric(YDirpoint) && isNumeric(ZDirpoint) && isNumeric(Radius))*/
                if (isNumeric(Xpoint) && isNumeric(Ypoint) && isNumeric(Radius))
                {
                    xConvertPoint = std::stod(Xpoint);
                    yConvertPoint = std::stod(Ypoint);
                    //zConvertPoint = std::stod(Zpoint);

                    /*xDirConvertPoint = std::stod(XDirpoint);
                    yDirConvertPoint = std::stod(YDirpoint);
                    zDirConvertPoint = std::stod(ZDirpoint);*/
                    xDirConvertPoint = 0;
                    yDirConvertPoint = 0;
                    zDirConvertPoint = 1;

                    RadiusConvert = std::stod(Radius);

                    if (create == false)
                    {
                        if (xDirConvertPoint != yDirConvertPoint || xDirConvertPoint != zDirConvertPoint || yDirConvertPoint != zDirConvertPoint)
                        {
                            Point = gp_Pnt(xConvertPoint, yConvertPoint, zConvertPoint);
                            Dir = gp_Dir(xDirConvertPoint, yDirConvertPoint, zDirConvertPoint);

                            gp_Circ gpCircle;
                            gpCircle.SetRadius(RadiusConvert);
                            gpCircle.SetLocation(Point);
                            gpCircle.SetAxis(gp_Ax1(Point, Dir));

                            TopoDS_Edge cicleEdge = BRepBuilderAPI_MakeEdge(gpCircle);
                            aisCicleEdge = new AIS_Shape(cicleEdge);
                            myContext->Display(aisCicleEdge, true);

                            create = true;

                        }

                    }

                }

                if (Xoldpoint != xConvertPoint || Yoldpoint != yConvertPoint || Zoldpoint != zConvertPoint ||
                    XDiroldpoint != xDirConvertPoint || YDiroldpoint != yDirConvertPoint || ZDiroldpoint != zDirConvertPoint ||
                    RadiusOld != RadiusConvert)
                {
                    myContext->Erase(aisCicleEdge, true);
                    create = false;
                    Xoldpoint = xConvertPoint;
                    Yoldpoint = yConvertPoint;
                    Zoldpoint = zConvertPoint;
                    XDiroldpoint = xDirConvertPoint;
                    YDiroldpoint = yDirConvertPoint;
                    ZDiroldpoint = zDirConvertPoint;
                    RadiusOld = RadiusConvert;
                }

                if (ImGui::Button("OK"))
                {
                    myContext->Erase(aisCicleEdge, true);
                    BuildPoint(Point);
                    BuildCircle(Point, Dir, RadiusConvert);
                    circleFlag = false;
                    //BuildLine(tempPoint1, tempPoint2); LineFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(aisCicleEdge, true);
                    circleFlag = false;
                    create = false;
                }

                ImGui::End();
            }
            ImGui::SameLine();
            if (ImGui::Button("Эллипс")) { ellipseFlag = true; }
            if (ellipseFlag)
            {
                static char Xpoint[10];
                static char Ypoint[10];
                static char Zpoint[10];
                static double xConvertPoint;
                static double yConvertPoint;
                static double zConvertPoint;
                static double Xoldpoint;
                static double Yoldpoint;
                static double Zoldpoint;

                static char XDirpoint[10];
                static char YDirpoint[10];
                static char ZDirpoint[10];
                static double xDirConvertPoint;
                static double yDirConvertPoint;
                static double zDirConvertPoint;
                static double XDiroldpoint;
                static double YDiroldpoint;
                static double ZDiroldpoint;

                static char MaxRadius[10];
                static double MaxRadiusConvert;
                static double MaxRadiusOld;
                static char MinRadius[10];
                static double MinRadiusConvert;
                static double MinRadiusOld;

                static gp_Pnt Point;
                static gp_Dir Dir;

                static Handle(AIS_Shape) aisElipsEdge;
                ImGui::Begin("Построить эллипс");
                ImGui::Columns(2, nullptr, false);

                ImGui::InputText("X1", Xpoint, IM_ARRAYSIZE(Xpoint));
                ImGui::InputText("Y1", Ypoint, IM_ARRAYSIZE(Ypoint));
                //ImGui::InputText("Z1", Zpoint, IM_ARRAYSIZE(Zpoint));
                ImGui::NextColumn();
                ImGui::InputText("Б.полуось", MaxRadius, IM_ARRAYSIZE(MaxRadius));
                ImGui::InputText("М.полуось", MinRadius, IM_ARRAYSIZE(MinRadius));

                if (isNumeric(Xpoint) && isNumeric(Ypoint) && isNumeric(MaxRadius) && isNumeric(MinRadius))
                {
                    xConvertPoint = std::stod(Xpoint);
                    yConvertPoint = std::stod(Ypoint);
                    //zConvertPoint = std::stod(Zpoint);

                    /*xDirConvertPoint = std::stod(XDirpoint);
                    yDirConvertPoint = std::stod(YDirpoint);
                    zDirConvertPoint = std::stod(ZDirpoint);*/
                    xDirConvertPoint = 0;
                    yDirConvertPoint = 0;
                    zDirConvertPoint = 1;

                    MaxRadiusConvert = std::stod(MaxRadius);
                    MinRadiusConvert = std::stod(MinRadius);

                    if (create == false)
                    {
                        if (xDirConvertPoint != yDirConvertPoint || xDirConvertPoint != zDirConvertPoint || yDirConvertPoint != zDirConvertPoint)
                        {
                            Point = gp_Pnt(xConvertPoint, yConvertPoint, zConvertPoint);
                            Dir = gp_Dir(xDirConvertPoint, yDirConvertPoint, zDirConvertPoint);

                            gp_Elips gpElipse;
                            gpElipse.SetLocation(Point);
                            gpElipse.SetMajorRadius(MaxRadiusConvert);
                            gpElipse.SetMinorRadius(MinRadiusConvert);
                            gpElipse.SetAxis(gp_Ax1(Point, Dir));

                            TopoDS_Edge ElipsEdge = BRepBuilderAPI_MakeEdge(gpElipse);
                            aisElipsEdge = new AIS_Shape(ElipsEdge);
                            myContext->Display(aisElipsEdge, true);

                            create = true;

                        }

                    }

                }

                if (Xoldpoint != xConvertPoint || Yoldpoint != yConvertPoint || Zoldpoint != zConvertPoint ||
                    XDiroldpoint != xDirConvertPoint || YDiroldpoint != yDirConvertPoint || ZDiroldpoint != zDirConvertPoint ||
                    MaxRadiusOld != MaxRadiusConvert || MinRadiusOld != MinRadiusConvert)
                {
                    myContext->Erase(aisElipsEdge, true);
                    create = false;
                    Xoldpoint = xConvertPoint;
                    Yoldpoint = yConvertPoint;
                    Zoldpoint = zConvertPoint;
                    XDiroldpoint = xDirConvertPoint;
                    YDiroldpoint = yDirConvertPoint;
                    ZDiroldpoint = zDirConvertPoint;
                    MaxRadiusOld = MaxRadiusConvert;
                    MinRadiusOld = MinRadiusConvert;
                }
                if (ImGui::Button("OK"))
                {
                    myContext->Erase(aisElipsEdge, true);
                    BuildPoint(Point);
                    BuildElips(Point, Dir, MaxRadiusConvert, MinRadiusConvert);
                    ellipseFlag = false;
                    //BuildLine(tempPoint1, tempPoint2); LineFlag = false;
                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(aisElipsEdge, true);
                    ellipseFlag = false;
                    create = false;
                }

                ImGui::End();
            }
            ImGui::SameLine();
            if (ImGui::Button("Дуга"))
            {
                ArcFlag = true;
                /*Handle(Geom_TrimmedCurve) cicle = GC_MakeArcOfCircle(gp_Pnt(0, 0, 0), gp_Pnt(5, 4, 0), gp_Pnt(10, 0, 0));
                TopoDS_Edge cicleEdge = BRepBuilderAPI_MakeEdge(cicle);
                Handle(AIS_Shape) aisCicleEdge = new AIS_Shape(cicleEdge);
                myContext->Display(aisCicleEdge, true);*/
            }
            if (ArcFlag)
            {
                static char Xpoint1[10];
                static char Ypoint1[10];
                static char Zpoint1[10];
                static double xConvertPoint1;
                static double yConvertPoint1;
                static double zConvertPoint1;
                static double Xoldpoint1;
                static double Yoldpoint1;
                static double Zoldpoint1;

                static char Xpoint2[10];
                static char Ypoint2[10];
                static char Zpoint2[10];
                static double xConvertPoint2;
                static double yConvertPoint2;
                static double zConvertPoint2;
                static double Xoldpoint2;
                static double Yoldpoint2;
                static double Zoldpoint2;

                static char Xpoint3[10];
                static char Ypoint3[10];
                static char Zpoint3[10];
                static double xConvertPoint3;
                static double yConvertPoint3;
                static double zConvertPoint3;
                static double Xoldpoint3;
                static double Yoldpoint3;
                static double Zoldpoint3;

                static gp_Pnt point1;
                static gp_Pnt point2;
                static gp_Pnt point3;

                static Handle(AIS_Shape) aisCicleEdge;

                ImGui::Begin("Построить дугу");
                ImGui::Columns(3, nullptr, false);

                ImGui::InputText("X1", Xpoint1, IM_ARRAYSIZE(Xpoint1));
                ImGui::InputText("Y1", Ypoint1, IM_ARRAYSIZE(Ypoint1));
                //ImGui::InputText("Z1", Zpoint1, IM_ARRAYSIZE(Zpoint1));

                ImGui::NextColumn();

                ImGui::InputText("X2", Xpoint2, IM_ARRAYSIZE(Xpoint2));
                ImGui::InputText("Y2", Ypoint2, IM_ARRAYSIZE(Ypoint2));
                //ImGui::InputText("Z2", Zpoint2, IM_ARRAYSIZE(Zpoint2));

                ImGui::NextColumn();

                ImGui::InputText("X3", Xpoint3, IM_ARRAYSIZE(Xpoint3));
                ImGui::InputText("Y3", Ypoint3, IM_ARRAYSIZE(Ypoint3));
                //ImGui::InputText("Z3", Zpoint3, IM_ARRAYSIZE(Zpoint3));

                /*if (isNumeric(Xpoint1) && isNumeric(Ypoint1) && isNumeric(Zpoint1) &&
                    isNumeric(Xpoint2) && isNumeric(Ypoint2) && isNumeric(Zpoint2) &&
                    isNumeric(Xpoint3) && isNumeric(Ypoint3) && isNumeric(Zpoint3))*/
                if (isNumeric(Xpoint1) && isNumeric(Ypoint1) &&
                    isNumeric(Xpoint2) && isNumeric(Ypoint2) &&
                    isNumeric(Xpoint3) && isNumeric(Ypoint3))
                {
                    xConvertPoint1 = std::stod(Xpoint1);
                    yConvertPoint1 = std::stod(Ypoint1);
                    //zConvertPoint1 = std::stod(Zpoint1);
                    zConvertPoint1 = 0;

                    xConvertPoint2 = std::stod(Xpoint2);
                    yConvertPoint2 = std::stod(Ypoint2);
                    //zConvertPoint2 = std::stod(Zpoint2);
                    zConvertPoint2 = 0;

                    xConvertPoint3 = std::stod(Xpoint3);
                    yConvertPoint3 = std::stod(Ypoint3);
                    //zConvertPoint3 = std::stod(Zpoint3);
                    zConvertPoint3 = 0;

                    if (create == false)
                    {
                        if ((xConvertPoint1 == xConvertPoint2 && yConvertPoint1 == yConvertPoint2) ||
                            (xConvertPoint1 == xConvertPoint3 && yConvertPoint1 == yConvertPoint3) ||
                            (xConvertPoint2 == xConvertPoint3 && yConvertPoint2 == yConvertPoint3)) {
                            // Есть хотя бы одна пара точек, которые совпадают
                            // Выполнить соответствующие действия
                        }
                        else {
                            point1 = gp_Pnt(xConvertPoint1, yConvertPoint1, zConvertPoint1);
                            point2 = gp_Pnt(xConvertPoint2, yConvertPoint2, zConvertPoint2);
                            point3 = gp_Pnt(xConvertPoint3, yConvertPoint3, zConvertPoint3);


                            Handle(Geom_TrimmedCurve) cicle = GC_MakeArcOfCircle(point1, point2, point3);
                            TopoDS_Edge cicleEdge = BRepBuilderAPI_MakeEdge(cicle);
                            aisCicleEdge = new AIS_Shape(cicleEdge);
                            myContext->Display(aisCicleEdge, true);

                            create = true;

                        }

                    }

                }
                if (Xoldpoint1 != xConvertPoint1 || Yoldpoint1 != yConvertPoint1 || Zoldpoint1 != zConvertPoint1 ||
                    Xoldpoint2 != xConvertPoint2 || Yoldpoint2 != yConvertPoint2 || Zoldpoint3 != zConvertPoint2 ||
                    Xoldpoint3 != xConvertPoint3 || Yoldpoint3 != yConvertPoint3 || Zoldpoint3 != zConvertPoint3)
                {
                    myContext->Erase(aisCicleEdge, true);
                    create = false;
                    Xoldpoint1 = xConvertPoint1;
                    Yoldpoint1 = yConvertPoint1;
                    Zoldpoint1 = zConvertPoint1;

                    Xoldpoint2 = xConvertPoint2;
                    Yoldpoint2 = yConvertPoint2;
                    Zoldpoint2 = zConvertPoint2;

                    Xoldpoint3 = xConvertPoint3;
                    Yoldpoint3 = yConvertPoint3;
                    Zoldpoint3 = zConvertPoint3;
                }
                if (ImGui::Button("OK"))
                {
                    myContext->Erase(aisCicleEdge, true);
                    BuildPoint(point1);
                    BuildPoint(point3);
                    BuildArc(point1, point2, point3);
                    ArcFlag = false;

                    create = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    myContext->Erase(aisCicleEdge, true);
                    ArcFlag = false;
                    create = false;
                }
                ImGui::End();
            }
            if (ImGui::Button("Закончить эскиз")) { CloseSketch(); SketchMode = false; }
            ImGui::SameLine();
            CheckCreateSketch = true;
            CheckSketch();
            if (CheckCreateSketch == false && ShapeBuild.size() != 0) { ImGui::Text("Эскиз не имеет замкнутый контур!"); }
        }
        ImGui::End();
    }

    //Окно история
    {
        // Установка размеров и расположения окна
        //С статус баром
        /*{
            ImVec2 screenSize = ImGui::GetIO().DisplaySize;
            ImVec2 windowSize(screenSize.x * 0.2f, screenSize.y * 0.75f);

            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, screenSize.y * 0.2f));
        }*/

        //Без статус бара
        {

            ImVec2 screenSize = ImGui::GetIO().DisplaySize;
            ImVec2 windowSize(screenSize.x * 0.23f, screenSize.y * 0.8f);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, screenSize.y * 0.2f));
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.8f);
            ImGui::Begin("History", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text(a);
            //if (ImGui::Button("1")) { if (ImGui::Button("2")); }
            ImGui::End();


        }

    }



    // Статус бар
    /*{
        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        ImVec2 windowSize(screenSize.x, screenSize.y * 0.05);

        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, screenSize.y * 0.95));

        ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("Это статус бар!");
        ImGui::End();
    }*/
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(myOcctWindow->getGlfwWindow());
}



// ================================================================
// Function : initDemoScene
// Purpose  :
// ================================================================
void GlfwOcctView::initDemoScene()
{
    if (myContext.IsNull())
    {
        return;
    }

    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_WIREFRAME);



    TCollection_AsciiString aGlInfo;
    {
        TColStd_IndexedDataMapOfStringString aRendInfo;
        myView->DiagnosticInformation(aRendInfo, Graphic3d_DiagnosticInfo_Basic);
        for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aRendInfo); aValueIter.More(); aValueIter.Next())
        {
            if (!aGlInfo.IsEmpty()) { aGlInfo += "\n"; }
            aGlInfo += TCollection_AsciiString("  ") + aValueIter.Key() + ": " + aValueIter.Value();
        }
    }
    Message::DefaultMessenger()->Send(TCollection_AsciiString("OpenGL info:\n") + aGlInfo, Message_Info);
}

// ================================================================
// Function : mainloop
// Purpose  :
// ================================================================
void GlfwOcctView::mainloop()
{
    while (!glfwWindowShouldClose(myOcctWindow->getGlfwWindow()))
    {
        // glfwPollEvents() for continuous rendering (immediate return if there are no new events)
        // and glfwWaitEvents() for rendering on demand (something actually happened in the viewer)
        //glfwPollEvents();
        glfwWaitEvents();
        if (!myView.IsNull())
        {
            FlushViewEvents(myContext, myView, Standard_True);

            renderGui();
            myView->Redraw();
        }
    }
}

// ================================================================
// Function : cleanup
// Purpose  :
// ================================================================
void GlfwOcctView::cleanup()
{
    // Cleanup IMGUI.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (!myView.IsNull())
    {
        myView->Remove();
    }
    if (!myOcctWindow.IsNull())
    {
        myOcctWindow->Close();
    }

    glfwTerminate();
}

// ================================================================
// Function : onResize
// Purpose  :
// ================================================================
void GlfwOcctView::onResize(int theWidth, int theHeight)
{
    if (theWidth != 0
        && theHeight != 0
        && !myView.IsNull())
    {
        myView->Window()->DoResize();
        myView->MustBeResized();
        myView->Invalidate();
        myView->Redraw();
    }
}

// ================================================================
// Function : onMouseScroll
// Purpose  :
// ================================================================
void GlfwOcctView::onMouseScroll(double theOffsetX, double theOffsetY)
{
    ImGuiIO& aIO = ImGui::GetIO();
    if (!myView.IsNull() && !aIO.WantCaptureMouse)
    {
        UpdateZoom(Aspect_ScrollDelta(myOcctWindow->CursorPosition(), int(theOffsetY * 8.0)));
    }
}

// ================================================================
// Function : onMouseButton
// Purpose  :
// ================================================================
void GlfwOcctView::onMouseButton(int theButton, int theAction, int theMods)
{
    ImGuiIO& aIO = ImGui::GetIO();
    if (myView.IsNull() || aIO.WantCaptureMouse)
    {
        return;
    }

    const Graphic3d_Vec2i aPos = myOcctWindow->CursorPosition();
    if (theAction == GLFW_PRESS)
    {
        PressMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
    else
    {
        ReleaseMouseButton(aPos, mouseButtonFromGlfw(theButton), keyFlagsFromGlfw(theMods), false);
    }
}

// ================================================================
// Function : onMouseMove
// Purpose  :
// ================================================================

void GlfwOcctView::onMouseMove(int thePosX, int thePosY)
{

    if (myView.IsNull())
    {
        return;
    }

    ImGuiIO& aIO = ImGui::GetIO();
    if (aIO.WantCaptureMouse)
    {
        myView->Redraw();
    }
    else
    {
        const Graphic3d_Vec2i aNewPos(thePosX, thePosY);
        UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), Standard_False);
    }

}
