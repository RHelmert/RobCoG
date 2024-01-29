#include "ManusClientSkeleton.h"

ClientSkeletonCollection::ClientSkeletonCollection()
{

}

ClientSkeletonCollection::ClientSkeletonCollection(const ClientSkeletonCollection& p_Original)
{
	CopyFrom(p_Original);
}

void ClientSkeletonCollection::CopyFrom(const ClientSkeletonCollection& p_Original)
{
	skeletons.resize(p_Original.skeletons.size());
	for (uint32_t i = 0; i < p_Original.skeletons.size(); i++)
	{
		skeletons[i].info.nodesCount = p_Original.skeletons[i].info.nodesCount;
		skeletons[i].info.id = p_Original.skeletons[i].info.id;

		skeletons[i].nodes = new SkeletonNode[p_Original.skeletons[i].info.nodesCount];
		for (uint32_t j = 0; j < p_Original.skeletons[i].info.nodesCount; j++)
		{
			skeletons[i].nodes[j].id = p_Original.skeletons[i].nodes[j].id;

			skeletons[i].nodes[j].transform.position.x = p_Original.skeletons[i].nodes[j].transform.position.x;
			skeletons[i].nodes[j].transform.position.y = p_Original.skeletons[i].nodes[j].transform.position.y;
			skeletons[i].nodes[j].transform.position.z = p_Original.skeletons[i].nodes[j].transform.position.z;

			skeletons[i].nodes[j].transform.rotation.x = p_Original.skeletons[i].nodes[j].transform.rotation.x;
			skeletons[i].nodes[j].transform.rotation.y = p_Original.skeletons[i].nodes[j].transform.rotation.y;
			skeletons[i].nodes[j].transform.rotation.z = p_Original.skeletons[i].nodes[j].transform.rotation.z;
			skeletons[i].nodes[j].transform.rotation.w = p_Original.skeletons[i].nodes[j].transform.rotation.w;

			skeletons[i].nodes[j].transform.scale.x = p_Original.skeletons[i].nodes[j].transform.scale.x;
			skeletons[i].nodes[j].transform.scale.y = p_Original.skeletons[i].nodes[j].transform.scale.y;
			skeletons[i].nodes[j].transform.scale.z = p_Original.skeletons[i].nodes[j].transform.scale.z;
		}
	}
}

bool ClientSkeletonCollection::CopySkeleton(uint32_t p_SkeletonId, ClientSkeleton* p_Data)
{
	for (size_t i = 0; i < skeletons.size(); i++)
	{
		if (skeletons[i].info.id == p_SkeletonId) 
		{
			p_Data->info.nodesCount = skeletons[i].info.nodesCount;
			p_Data->info.id = skeletons[i].info.id;
			p_Data->info.publishTime = skeletons[i].info.publishTime;
			p_Data->nodes = new SkeletonNode[skeletons[i].info.nodesCount];
			for (uint32_t j = 0; j < skeletons[i].info.nodesCount; j++)
			{
				p_Data->nodes[j].id = skeletons[i].nodes[j].id;

				p_Data->nodes[j].transform.position.x = skeletons[i].nodes[j].transform.position.x;
				p_Data->nodes[j].transform.position.y = skeletons[i].nodes[j].transform.position.y;
				p_Data->nodes[j].transform.position.z = skeletons[i].nodes[j].transform.position.z;

				p_Data->nodes[j].transform.rotation.x = skeletons[i].nodes[j].transform.rotation.x;
				p_Data->nodes[j].transform.rotation.y = skeletons[i].nodes[j].transform.rotation.y;
				p_Data->nodes[j].transform.rotation.z = skeletons[i].nodes[j].transform.rotation.z;
				p_Data->nodes[j].transform.rotation.w = skeletons[i].nodes[j].transform.rotation.w;

				p_Data->nodes[j].transform.scale.x = skeletons[i].nodes[j].transform.scale.x;
				p_Data->nodes[j].transform.scale.y = skeletons[i].nodes[j].transform.scale.y;
				p_Data->nodes[j].transform.scale.z = skeletons[i].nodes[j].transform.scale.z;
			}
			return true;
		}
	}
	return false;
}