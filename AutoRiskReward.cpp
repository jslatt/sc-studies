#include "sierrachart.h"
#include <string>
SCDLLName("RKornmeyer - Auto Position Sizing")

/*
    Updated by Rkornmeyer orignally Written by Frozen Tundra
*/


SCSFExport scsf_AutoRiskReward(SCStudyInterfaceRef sc)
{
    // logging object
    SCString msg;
    

    // helper class
    //Helper help(sc);

    SCInputRef i_FontSize = sc.Input[0];
    SCInputRef i_LineSize = sc.Input[1];
    SCInputRef i_MarkerSize = sc.Input[2];
    SCInputRef i_UseBold = sc.Input[3];
    SCInputRef i_TransparentBg = sc.Input[4];
    SCInputRef i_RemoveLatestDrawing = sc.Input[5];
    SCInputRef i_Risk = sc.Input[6];
    SCInputRef i_TargetR = sc.Input[7];


    // Set configuration variables
    if (sc.SetDefaults)
    {
        sc.GraphName = "Auto Risk Reward Tool";
        sc.StudyDescription = "";
        sc.GraphRegion = 0;
        sc.GraphShortName = "AutoRR";
        sc.GraphRegion = 0;

        i_FontSize.Name = "Font Size";
        i_FontSize.SetInt(14);

        i_LineSize.Name = "Line Size";
        i_LineSize.SetInt(2);

        i_MarkerSize.Name = "Marker Size";
        i_MarkerSize.SetInt(8);

        i_UseBold.Name = "Bold Text?";
        i_UseBold.SetYesNo(1);

        i_TransparentBg.Name = "Transparent Background?";
        i_TransparentBg.SetYesNo(0);

        i_Risk.Name = "Risk";
        i_Risk.SetInt(125);

        i_TargetR.Name = "Set The Target R Value";
        i_TargetR.SetFloat(2.0);

        sc.AutoLoop = 1;
        sc.FreeDLL = 1;

        return;
    }

    //persistent vars for menu
    int& r_MenuID = sc.GetPersistentInt(1);
    int& r_MenuID2 = sc.GetPersistentInt(2);
    int& r_CheckedMenuID = sc.GetPersistentInt(3);

    // persistent vars for when Sim orders disappear from Trade Orders window
    // (Guitarmadillo: this doesn't happen with live orders )

    double& EntryIndex = sc.GetPersistentDouble(4);
    double& EntryPrice = sc.GetPersistentDouble(5);
    double& StopPrice = sc.GetPersistentDouble(6);
    double& TargetPrice = sc.GetPersistentDouble(7);
    int& LineNumber = sc.GetPersistentInt(8);
    int& r_SeparatorMenuID = sc.GetPersistentInt(9);
    int& r_CheckedState = sc.GetPersistentInt(10);
    double& TickRisk = sc.GetPersistentDouble(11);
    int& CurrentLotSize = sc.GetPersistentInt(12);
    int PriorLotSize = 1;

    if (sc.LastCallToFunction)
    {
        // Remove menu items when study is removed
        sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, r_MenuID);
        sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, r_MenuID2);
        sc.RemoveACSChartShortcutMenuItem(sc.ChartNumber, r_SeparatorMenuID);
        sc.DeleteUserDrawnACSDrawing(sc.ChartNumber, LineNumber);
        sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, LineNumber);

        return;
    }


    if (sc.UpdateStartIndex == 0)
    {

        // add chart short cut menu item if not already added (persist var initialized to zero, negative previous fail)
        if (r_MenuID <= 0)
        {
            r_MenuID = sc.AddACSChartShortcutMenuItem(sc.ChartNumber, "Set Stop");
            r_MenuID2 = sc.AddACSChartShortcutMenuItem(sc.ChartNumber, "Clear All");
            // add a menu separator
            r_SeparatorMenuID = sc.AddACSChartShortcutMenuSeparator(sc.ChartNumber);
        }      

    }

    int BarIndex = sc.ArraySize - 1;

    // wait for an event
    if (sc.MenuEventID != 0)
    {
        if (sc.MenuEventID == r_MenuID)
        {
            //msg.AppendFormat("Got menu event id %i, ", sc.MenuEventID);
            StopPrice = sc.ChartTradingOrderPrice;                  
        }
        else if (sc.MenuEventID == r_MenuID2) 
        {
            sc.DeleteACSChartDrawing(sc.ChartNumber, TOOL_DELETE_CHARTDRAWING, LineNumber);
            StopPrice = 0;
        }
    }

    CurrentLotSize = sc.TradeWindowOrderQuantity;

    //msg.AppendFormat("Lot Size %i, %i, ", CurrentLotSize, PriorLotSize);

    TickRisk = (i_Risk.GetInt() / sc.CurrencyValuePerTick) / CurrentLotSize;

    if (StopPrice > sc.Close[sc.Index])
    {
        EntryPrice = StopPrice - (TickRisk * sc.TickSize);
        TargetPrice = EntryPrice - ((TickRisk * sc.TickSize)* i_TargetR.GetFloat());

    }
    else if (StopPrice < sc.Close[sc.Index])
    {
        EntryPrice = StopPrice + (TickRisk * sc.TickSize);
        TargetPrice = EntryPrice + ((TickRisk * sc.TickSize) * i_TargetR.GetFloat());
    }    
        
    if (EntryPrice <= 0 || StopPrice <= 0 || TargetPrice <= 0)
     {
         return;
     }


    


    // draw the Risk Reward Tool onto the chart
    s_UseTool Tool;

    Tool.Clear();
    Tool.ChartNumber = sc.ChartNumber;

    if (LineNumber != 0)
        Tool.LineNumber = LineNumber;


    Tool.Region = 0;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    //Tool.AddAsUserDrawnDrawing = 1;

    Tool.DrawingType = DRAWING_REWARD_RISK;

    Tool.BeginIndex = BarIndex;
    Tool.BeginValue = StopPrice;
    Tool.EndIndex = BarIndex;
    Tool.EndValue = EntryPrice;
    Tool.ThirdIndex = BarIndex;
    Tool.ThirdValue = TargetPrice;

    Tool.Color = COLOR_YELLOW;                // text color
    Tool.TransparentLabelBackground = i_TransparentBg.GetInt();    // opaque labels
    Tool.TextAlignment = DT_RIGHT;            // text alignment (target marker is left/right of text)
    Tool.ShowTickDifference = 1;            // text options
    Tool.ShowPriceDifference = 1;
    Tool.ShowCurrencyValue = 1;
    Tool.FontFace = sc.GetChartTextFontFaceName();    // override chart drawing text font
    Tool.FontSize = i_FontSize.GetInt();
    Tool.FontBold = i_UseBold.GetInt();

    // remaining options are controlled via the use tool levels
    Tool.LevelColor[0] = RGB(240,33,22);            // stop to entry line
    Tool.LevelStyle[0] = LINESTYLE_SOLID;
    Tool.LevelWidth[0] = i_LineSize.GetInt();

    Tool.LevelColor[1] = RGB(13,166,240);        // entry to target line
    Tool.LevelStyle[1] = LINESTYLE_SOLID;
    Tool.LevelWidth[1] = i_LineSize.GetInt();

    Tool.LevelColor[2] = RGB(192,192,192);            // entry marker
    Tool.LevelStyle[2] = MARKER_DASH;            // marker type
    Tool.RetracementLevels[2] = i_MarkerSize.GetInt();                // marker size
    Tool.LevelWidth[2] = i_LineSize.GetInt();                        // marker width

    Tool.LevelColor[3] = RGB(240, 33, 22);                // stop marker
    Tool.LevelStyle[3] = MARKER_DASH;            // marker type
    Tool.RetracementLevels[3] = i_MarkerSize.GetInt();                // marker size
    Tool.LevelWidth[3] = i_LineSize.GetInt();                        // marker width

    Tool.LevelColor[4] = RGB(13, 166, 240);            // target marker
    Tool.LevelStyle[4] = MARKER_DASH;            // marker type
    Tool.RetracementLevels[4] = i_MarkerSize.GetInt();                // marker size
    Tool.LevelWidth[4] = i_LineSize.GetInt();                        // marker width

    sc.UseTool(Tool);  // here we make the function call to add the reward risk tool
    LineNumber = Tool.LineNumber; // remember line number which has been automatically set
    //sc.AddMessageToLog(msg, 0);
   
}

