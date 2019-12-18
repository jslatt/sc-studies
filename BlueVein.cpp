// BlueVeins Custom Studies
// https://bluve.in or https://t.me/bluevein

#include "sierrachart.h"

SCDLLName("BlueVeins Studies")


// Track Target Location
SCSFExport scsf_OrderTracker(SCStudyInterfaceRef sc)
{
	//Define references to the Subgraphs and Inputs for easy reference
	SCSubgraphRef TargetSubgraph = sc.Subgraph[0];
	SCSubgraphRef StopSubgraph = sc.Subgraph[1];

	if (sc.SetDefaults)
	{
		sc.GraphName = "Target and Stop Tracker";

		TargetSubgraph.Name = "Target";
		TargetSubgraph.DrawStyle = DRAWSTYLE_STAIR_STEP;

		StopSubgraph.Name = "Stop";
		StopSubgraph.DrawStyle = DRAWSTYLE_STAIR_STEP;

		sc.AutoLoop = 1;
		sc.GraphRegion = 0;

		return;
	}

	s_SCTradeOrder Order;

	int Result;

	Result = sc.GetNearestStopOrder(Order);

	if(Result > 0 && Order.OrderStatusCode == SCT_OSC_OPEN)
		StopSubgraph[sc.Index] = Order.Price1;

	Result = sc.GetNearestTargetOrder(Order);

	if(Result > 0 && Order.OrderStatusCode == SCT_OSC_OPEN)
		TargetSubgraph[sc.Index] = Order.Price1;
}
