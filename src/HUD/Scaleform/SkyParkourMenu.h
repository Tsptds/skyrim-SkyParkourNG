#pragma once
#include "_References/RuntimeVariables.h"

namespace Scaleform {
    class SkyParkourMenu : public RE::IMenu {
        public:
            static constexpr std::string_view MENU_NAME = "SkyParkour";
            static constexpr std::int8_t SORT_PRIORITY = 0;

            enum IndicatorType : uint8_t { kInvisible = 0, kClimb, kVault, kOutOfStamina };

            // Factory
            static RE::stl::owner<RE::IMenu*> Creator() {
                return new SkyParkourMenu();
            }

            // Register menu
            static void Register() {
                RE::UI::GetSingleton()->Register(MENU_NAME.data(), Creator);
                LOG("Indicator Menu Registered");
            }

            // Check if open
            bool IsOpen() const {
                return _bIsOpen;
            }

            void SetActiveIndicatorType(IndicatorType type) {
                if (!_view)
                    return;
                if (_lastIndic == type)
                    return;

                _lastIndic = type;
                RE::GFxValue arg(static_cast<int>(type));
                _view->Invoke("_root.SetIndicatorType", nullptr, &arg, 1);
            }

            // Get GFx view
            RE::GPtr<RE::GFxMovieView> GetView() const {
                return _view;
            }

            // Show the menu itself, keep this on at all times
            void Show() {
                auto queue = RE::UIMessageQueue::GetSingleton();
                if (queue) {
                    queue->AddMessage(MENU_NAME.data(), RE::UI_MESSAGE_TYPE::kShow, nullptr);
                    LOG("Notified UI to show SkyParkourMenu");
                }
            }

            // Should never be called at all
            void Hide() {
                auto queue = RE::UIMessageQueue::GetSingleton();
                if (queue) {
                    queue->AddMessage(MENU_NAME.data(), RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    LOG("Notified UI to hide SkyParkourMenu");
                }
            }

        protected:
            using UIResult = RE::UI_MESSAGE_RESULTS;
            SkyParkourMenu() {
                LOG("Creating SkyParkour Menu");
                auto menu = static_cast<Super*>(this);
                menu->depthPriority = SORT_PRIORITY;
                auto scaleformManager = RE::BSScaleformManager::GetSingleton();

                // do NOT use LoadMovieEx if you want skse scaleform hooks, it doesn't call the original function which is hooked by skse
                [[maybe_unused]] const auto success = scaleformManager->LoadMovieEx(menu, MENU_NAME, [](RE::GFxMovieDef* a_def) -> void {
                    a_def->SetState(RE::GFxState::StateType::kLog, RE::make_gptr<Logger>().get());
                });

                // this works instead
                //[[maybe_unused]] const auto success = scaleformManager->LoadMovie(menu, menu->uiMovie, FILE_NAME.data());
                assert(success);
                if (menu && menu->uiMovie) {
                    auto def = menu->uiMovie->GetMovieDef();
                    if (def) {
                        def->SetState(RE::GFxState::StateType::kLog, RE::make_gptr<Logger>().get());
                    }
                }

                _bIsOpen = false;
                // _movieLastTime = std::chrono::steady_clock::now();

                depthPriority = SORT_PRIORITY;
                menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);
                menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
                menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);
                // menuFlags.set(RE::UI_MENU_FLAGS::kRendersUnderPauseMenu);

                /*-- TEST STUFF --*/
                // menuFlags.set(RE::UI_MENU_FLAGS::kPausesGame);
                // menuFlags.set(RE::UI_MENU_FLAGS::kUsesCursor);
                /*-- END OF TEST STUFF --*/

                menu->inputContext = RE::IMenu::Context::kNone;
                _view = menu->uiMovie;
            }

            SkyParkourMenu(const SkyParkourMenu&) = default;
            SkyParkourMenu(SkyParkourMenu&&) = default;
            ~SkyParkourMenu() = default;

            // PostCreate called by engine after menu is constructed
            void PostCreate() override {
                _bIsOpen = true;
                RE::GRectF rect = _view->GetVisibleFrameRect();
                _screenRes.x = fabs(rect.left - rect.right);
                _screenRes.y = fabs(rect.top - rect.bottom);

                Super::PostCreate();
                LOG("Menu Created");
                LOG("|>_SkyParkour Loaded_<|");
            }

            // Handle show/hide messages
            UIResult ProcessMessage(RE::UIMessage& a_message) override {
                using Type = RE::UI_MESSAGE_TYPE;

                switch (*a_message.type) {
                    case Type::kShow:
                        _bIsOpen = true;
                        LOG("Menu shown");
                        return Super::ProcessMessage(a_message);

                    case Type::kHide:
                        /* Simply ignore hide requests, no reason to hide. Indicator States are synced to parkour states */
                        // LOG("UI HIDDEN");
                        // _bIsOpen = false;
                        // return Super::ProcessMessage(a_message);
                        return RE::UI_MESSAGE_RESULTS::kIgnore;

                    default:
                        return Super::ProcessMessage(a_message);
                }
            }

            // Advance the movie each frame
            void AdvanceMovie(float, std::uint32_t) override {
                if (!_view)
                    return;

                if (RuntimeVariables::IsMenuOpen || !RuntimeVariables::IsParkourActive) {
                    if (_lastIndic != IndicatorType::kInvisible) {
                        SetActiveIndicatorType(IndicatorType::kInvisible);
                    }
                    return;
                }

                _view->Advance(1.0f / 60.0f);  // Advance at ~60 FPS

                auto targetPos = WorldToScreen(RuntimeVariables::ledgePoint);
                ClampToScreen(targetPos);

                if (_lastScreenPosition.x < 0.f && _lastScreenPosition.y < 0.f) {
                    _lastScreenPosition = targetPos;
                }

                RE::NiPoint2 _velocity{0.f, 0.f};
                constexpr float damping = 0.3f;          // 0 = very soft, 1 = very stiff
                constexpr float smoothingFactor = 0.2f;  // how fast it follows the target

                // Compute the difference and apply damping
                RE::NiPoint2 delta = targetPos - _lastScreenPosition;
                _velocity = (_velocity + delta * smoothingFactor) * damping;

                _lastScreenPosition += _velocity;

                RE::GFxValue args[2];
                args[0] = RE::GFxValue(static_cast<int>(_lastScreenPosition.x));
                args[1] = RE::GFxValue(static_cast<int>(_lastScreenPosition.y));

                _view->Invoke("_root.SetScreenPosition", nullptr, args, 2);
            }

            RE::NiPoint2 WorldToScreen(const RE::NiPoint3& a_worldPos) const {
                RE::NiPoint2 screenPos;
                float depth;

                RE::NiCamera::WorldPtToScreenPt3((float (*)[4]) g_worldToCamMatrix, *g_viewPort, a_worldPos, screenPos.x, screenPos.y,
                                                 depth, 1e-5f);
                RE::GRectF rect = _view->GetVisibleFrameRect();

                screenPos.x = rect.left + (rect.right - rect.left) * screenPos.x;
                screenPos.y = 1.f - screenPos.y;
                screenPos.y = rect.top + (rect.bottom - rect.top) * screenPos.y;

                return screenPos;
            }

            void ClampToScreen(RE::NiPoint2& a_point) {
                constexpr float maxOvershoot_Top = -5.f;
                constexpr float maxOvershoot_Bot = -20.f;

                /* Horizontal is buggy for some reason, probably windows problems idk */
                // if (a_point.x < 0.f) {
                //     float overshootX = fabs(a_point.x);
                //     if (overshootX > maxOvershoot) {
                //         a_point.x += overshootX - maxOvershoot;
                //     }
                // }
                // else if (a_point.x > _screenRes.x) {
                //     float overshootX = a_point.x - _screenRes.x;
                //     if (overshootX > maxOvershoot) {
                //         a_point.x -= overshootX - maxOvershoot;
                //     }
                // }

                if (a_point.y < 0.f) {
                    float overshootY = fabs(a_point.y);
                    if (overshootY > maxOvershoot_Top) {
                        a_point.y += overshootY - maxOvershoot_Top;
                    }
                }
                else if (a_point.y > _screenRes.y) {
                    float overshootY = a_point.y - _screenRes.y;
                    if (overshootY > maxOvershoot_Bot) {
                        a_point.y -= overshootY - maxOvershoot_Bot;
                    }
                }
            }

        private:
            inline static uintptr_t g_worldToCamMatrix = RELOCATION_ID(519579, 406126).address();
            inline static RE::NiRect<float>* g_viewPort = (RE::NiRect<float>*) RELOCATION_ID(519618, 406160).address();
            using Super = RE::IMenu;
            bool _bIsOpen;

            RE::NiPoint2 _screenRes;
            RE::NiPoint2 _lastScreenPosition{-1.f, -1.f};

            RE::GPtr<RE::GFxMovieView> _view;
            // std::chrono::steady_clock::time_point _movieLastTime;
            IndicatorType _lastIndic;

            class Logger : public RE::GFxLog {
                public:
                    void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override {
                        std::string fmt(a_fmt ? a_fmt : "");
                        while (!fmt.empty() && fmt.back() == '\n') {
                            fmt.pop_back();
                        }

                        std::va_list args;
                        va_copy(args, a_argList);
                        std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
                        std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
                        va_end(args);

                        LOG("{}: {}"sv, SkyParkourMenu::MENU_NAME, buf.data());
                    }
            };
    };
}  // namespace Scaleform
