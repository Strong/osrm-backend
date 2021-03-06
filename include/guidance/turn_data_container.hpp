#ifndef OSRM_GUIDANCE_TURN_DATA_CONTAINER_HPP
#define OSRM_GUIDANCE_TURN_DATA_CONTAINER_HPP

#include "extractor/travel_mode.hpp"
#include "guidance/turn_bearing.hpp"
#include "guidance/turn_instruction.hpp"

#include "storage/io_fwd.hpp"
#include "storage/shared_memory_ownership.hpp"

#include "util/vector_view.hpp"

#include "util/typedefs.hpp"

namespace osrm
{
namespace guidance
{
namespace detail
{
template <storage::Ownership Ownership> class TurnDataContainerImpl;
}

namespace serialization
{
template <storage::Ownership Ownership>
void read(storage::io::FileReader &reader,
          detail::TurnDataContainerImpl<Ownership> &turn_data,
          std::uint32_t &connectivity_checksum);

template <storage::Ownership Ownership>
void write(storage::io::FileWriter &writer,
           const detail::TurnDataContainerImpl<Ownership> &turn_data,
           const std::uint32_t connectivity_checksum);
}

struct TurnData
{
    TurnInstruction turn_instruction;
    LaneDataID lane_data_id;
    EntryClassID entry_class_id;
    TurnBearing pre_turn_bearing;
    TurnBearing post_turn_bearing;
};

namespace detail
{
template <storage::Ownership Ownership> class TurnDataContainerImpl
{
    template <typename T> using Vector = util::ViewOrVector<T, Ownership>;

  public:
    TurnDataContainerImpl() = default;

    TurnDataContainerImpl(Vector<TurnInstruction> turn_instructions,
                          Vector<LaneDataID> lane_data_ids,
                          Vector<EntryClassID> entry_class_ids,
                          Vector<TurnBearing> pre_turn_bearings,
                          Vector<TurnBearing> post_turn_bearings)
        : turn_instructions(std::move(turn_instructions)), lane_data_ids(std::move(lane_data_ids)),
          entry_class_ids(std::move(entry_class_ids)),
          pre_turn_bearings(std::move(pre_turn_bearings)),
          post_turn_bearings(std::move(post_turn_bearings))
    {
    }

    EntryClassID GetEntryClassID(const EdgeID id) const { return entry_class_ids[id]; }

    TurnBearing GetPreTurnBearing(const EdgeID id) const { return pre_turn_bearings[id]; }

    TurnBearing GetPostTurnBearing(const EdgeID id) const { return post_turn_bearings[id]; }

    LaneDataID GetLaneDataID(const EdgeID id) const { return lane_data_ids[id]; }

    bool HasLaneData(const EdgeID id) const { return INVALID_LANE_DATAID != lane_data_ids[id]; }

    guidance::TurnInstruction GetTurnInstruction(const EdgeID id) const
    {
        return turn_instructions[id];
    }

    // Used by EdgeBasedGraphFactory to fill data structure
    template <typename = std::enable_if<Ownership == storage::Ownership::Container>>
    void push_back(const TurnData &data)
    {
        turn_instructions.push_back(data.turn_instruction);
        lane_data_ids.push_back(data.lane_data_id);
        entry_class_ids.push_back(data.entry_class_id);
        pre_turn_bearings.push_back(data.pre_turn_bearing);
        post_turn_bearings.push_back(data.post_turn_bearing);
    }

    template <typename = std::enable_if<Ownership == storage::Ownership::Container>>
    void append(const std::vector<TurnData> &others)
    {
        std::for_each(
            others.begin(), others.end(), [this](const TurnData &other) { push_back(other); });
    }

    friend void serialization::read<Ownership>(storage::io::FileReader &reader,
                                               TurnDataContainerImpl &turn_data_container,
                                               std::uint32_t &connectivity_checksum);
    friend void serialization::write<Ownership>(storage::io::FileWriter &writer,
                                                const TurnDataContainerImpl &turn_data_container,
                                                const std::uint32_t connectivity_checksum);

  private:
    Vector<TurnInstruction> turn_instructions;
    Vector<LaneDataID> lane_data_ids;
    Vector<EntryClassID> entry_class_ids;
    Vector<TurnBearing> pre_turn_bearings;
    Vector<TurnBearing> post_turn_bearings;
};
}

using TurnDataExternalContainer = detail::TurnDataContainerImpl<storage::Ownership::External>;
using TurnDataContainer = detail::TurnDataContainerImpl<storage::Ownership::Container>;
using TurnDataView = detail::TurnDataContainerImpl<storage::Ownership::View>;
}
}

#endif
