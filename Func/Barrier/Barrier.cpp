#include "Barrier.h"

/// <summary>
/// TransitionBarrierを張る
/// </summary>
/// <param name="resource"></param>
/// <param name="commandList"></param>
/// <param name="stateBefore"></param>
/// <param name="stateAfter"></param>
void TransitionBarrier(ID3D12Resource* resource, ID3D12GraphicsCommandList* commandList , D3D12_RESOURCE_STATES stateBefore , D3D12_RESOURCE_STATES stateAfter)
{
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};

	// TransitionBarrier
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;


	// barrierを張る対象のResource
	barrier.Transition.pResource = resource;

	// 遷移前（現在）のResourceState
	barrier.Transition.StateBefore = stateBefore;

	// 遷移後のResourceState
	barrier.Transition.StateAfter = stateAfter;

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
}