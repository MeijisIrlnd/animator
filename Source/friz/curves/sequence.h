/*
 * Copyright (c) 2019 Brett g Porter.
 */

#pragma once

#include "../control/animation.h"

namespace friz
{
/**
 * @class Sequence
 *
 * @brief An Animation class that can hold multiple Animation objects and
 *        execute them in sequence. An easy way to create complex effects
 *        from primitive movements.
 */

template <int valueCount> class Sequence : public Animation<valueCount>
{
public:
    Sequence (int id = 0)
    : Animation<valueCount> (id)
    , currentEffect { 0 }
    {
    }

    bool isFinished () override { return (currentEffect >= sequence.size ()); }

    bool isReady () const override
    {
        for (const auto& effect : sequence)
        {
            if ((nullptr == effect) || (!effect->isReady ()))
                return false;
        }
        return true;
    }

    AnimationType::Status update () override
    {
        auto effect = getEffect (currentEffect);
        if (effect)
        {
            if (AnimationType::Status::finished == effect->update ())
                ++currentEffect;

            return isFinished () ? AnimationType::Status::finished
                                 : AnimationType::Status::processing;
        }

        return AnimationType::Status::finished;
    }

    void cancel (bool moveToEndPosition) override
    {
        if (moveToEndPosition)
        {
            auto lastIndex  = static_cast<int> (sequence.size () - 1);
            auto lastEffect = getEffect (lastIndex);
            if (lastEffect)
                lastEffect->cancel (moveToEndPosition);
        }
    }

    void addAnimation (std::unique_ptr<Animation<valueCount>> effect)
    {
        // We need to make each of the effects notify us on update or completion,
        // so that we can pass those along to whoever passed in a single
        // lambda to us.
        effect->onUpdate (
            [this] (int /*id*/, const typename Animation<valueCount>::ValueList& val)
            {
                if (this->updateFn != nullptr)
                    this->updateFn (this->getId (), val);
            });

        effect->onCompletion (
            [this] (int /*id*/)
            {
                // Each effect in the sequence will notify us, but we only pass
                // along the final one.
                if ((currentEffect == sequence.size () - 1) &&
                    this->completionFn != nullptr)
                    this->completionFn (this->getId ());
            });

        sequence.push_back (std::move (effect));
    }

private:
    /**
     * Get a pointer to one of our effects by its index.
     * @param  index 0..size-1
     * @return       nullptr if index is out of range.
     */
    Animation<valueCount>* getEffect (int index)
    {
        if (juce::isPositiveAndBelow (index, sequence.size ()))
            return sequence[index].get ();

        jassertfalse;
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<Animation<valueCount>>> sequence;

    int currentEffect { 0 };
};

} // namespace friz
