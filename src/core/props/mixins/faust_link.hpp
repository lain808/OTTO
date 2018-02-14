#pragma once

#include <vector>

#include "util/utility.hpp"

#include "../internal/mixin_macros.hpp"
#include "../internal/property.hpp"

#include "has_value.hpp"

#include "core/audio/faust.hpp"

#include "services/logger.hpp"

namespace otto::core::props {

  OTTO_PROPS_MIXIN(faust_link, REQUIRES(faust_link, has_value));

  using FaustLink = audio::FaustLink;

  template<>
  struct mixin::leaf_interface<faust_link> {
    const std::function<void()> refresh_links;
    const std::function<void(FaustLink)> register_link;
    const std::string& name;
  };

  template<>
  struct mixin::branch_interface<faust_link> : BaseBranchInterface<faust_link> {
    void refresh_links()
    {
      for (auto&& itf : children()) {
        if (itf.is_branch()) {
          itf.get_branch().refresh_links();
        } else {
          itf.get_leaf().refresh_links();
        }
      }
    }

    template<typename Container>
    void register_link(const Container& name_parts, FaustLink link) {
      auto first = std::begin(name_parts), last = std::end(name_parts);
      auto found = util::find_if(children(), [& name = *first](
                                               auto&& child) {
        return (child.is_leaf() ? child.get_leaf().name
                                : child.get_branch().name()) == name;
      });
      if (found != children().end()) {
        if (found->is_leaf()) {
          if (std::next(first)  == last) {
            found->get_leaf().register_link(link);
          } else {
            LOGE("Expected a branch property, got a leaf");
          }
        } else {
          if (std::next(first) != last) {
            found->get_branch().register_link(
              util::sequence(std::move(first++), std::move(last)));
          } else {
            LOGE("Expected a leaf property, got a branch");
          }
        }
      }
    }
  };

  OTTO_PROPS_MIXIN_IMPL(faust_link) {
    OTTO_PROPS_MIXIN_DECLS(faust_link);

    void init(FaustLink::Type type) {
      type_ = type;
    }

    void refresh_links()
    {
      for (auto&& fl : faust_links_) {
        fl.refresh();
      }
    }

    void add_link(const FaustLink& fl)
    {
      faust_links_.push_back(fl);
    }

    void clear_links()
    {
      faust_links_.clear();
    }

    void on_hook(hook<has_value::hooks::on_set, HookOrder::After> & hook)
    {
      for (auto&& fl : faust_links_) {
        if (fl.type == FaustLink::Type::ToFaust) {
          fl.update(hook.value());
        }
      }
    }

    interface_type& interface() noexcept
    {
      return interface_;
    }

    const FaustLink::Type& type = type_;

  private:
    FaustLink::Type type_;
    std::vector<float*> faust_links_;

    interface_type interface_ = {
      util::capture_this(&self_type::refresh_links, this)};
  };

} // namespace otto::core::props::mixins